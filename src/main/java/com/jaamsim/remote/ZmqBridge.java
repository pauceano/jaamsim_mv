/*
 * JaamSim Discrete Event Simulation
 * Copyright (C) 2026 JaamSim Software Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.jaamsim.remote;

import java.util.ArrayList;
import java.util.HashMap;

import org.zeromq.SocketType;
import org.zeromq.ZContext;
import org.zeromq.ZMQ;

import com.jaamsim.Graphics.DisplayEntity;
import com.jaamsim.JSON.JSONValue;
import com.jaamsim.JSON.JSONWriter;
import com.jaamsim.basicsim.Entity;
import com.jaamsim.basicsim.JaamSimModel;
import com.jaamsim.basicsim.Log;
import com.jaamsim.basicsim.LogListener;
import com.jaamsim.basicsim.RunListener;
import com.jaamsim.input.InputAgent;
import com.jaamsim.input.ValueHandle;
import com.jaamsim.math.Vec3d;
import com.jaamsim.states.StateEntity;

public class ZmqBridge implements LogListener, RunListener {

	private ZContext context;
	private ZMQ.Socket pubSocket;
	private ZMQ.Socket routerSocket;
	private Thread pollerThread;
	private Thread commandThread;
	private volatile boolean running;
	private JaamSimModel simModel;
	private int lastLogIndex;

	public ZmqBridge() {
		context = new ZContext();
		lastLogIndex = 0;
	}

	public void start(JaamSimModel model) {
		simModel = model;
		running = true;

		pubSocket = context.createSocket(SocketType.PUB);
		pubSocket.bind(ZmqConfig.PUB_ADDRESS);

		routerSocket = context.createSocket(SocketType.ROUTER);
		routerSocket.bind(ZmqConfig.ROUTER_ADDRESS);

		simModel.addRunListener(this);
		Log.addListener(this);

		pollerThread = new Thread(this::pollerLoop, "ZmqPoller");
		pollerThread.setDaemon(true);
		pollerThread.start();

		commandThread = new Thread(this::commandLoop, "ZmqCommand");
		commandThread.setDaemon(true);
		commandThread.start();
	}

	public void stop() {
		running = false;
		if (pollerThread != null) {
			try { pollerThread.join(2000); } catch (InterruptedException e) {}
		}
		if (commandThread != null) {
			try { commandThread.join(2000); } catch (InterruptedException e) {}
		}
		context.close();
	}

	@Override
	public void update() {
		ArrayList<String> lines = Log.getLog(lastLogIndex);
		for (String line : lines) {
			publishLogLine(line);
		}
		lastLogIndex += lines.size();
	}

	@Override
	public void runEnded() {
		publishEvent("runEnded");
	}

	@Override
	public void handleRuntimeError(JaamSimModel sm, Throwable t) {
		String msg = t.getMessage() != null ? t.getMessage() : t.getClass().getSimpleName();
		publishEvent("runError", msg);
	}

	private void publishEvent(String type) {
		publishEvent(type, "");
	}

	private void publishEvent(String type, String data) {
		HashMap<String, JSONValue> map = new HashMap<>();
		map.put("type", JSONValue.makeStringVal(type));
		map.put("data", JSONValue.makeStringVal(data));
		JSONValue msg = new JSONValue();
		msg.mapVal = map;
		publish("runEvent", JSONWriter.writeJSONValue(msg));
	}

	private void publishLogLine(String line) {
		HashMap<String, JSONValue> map = new HashMap<>();
		map.put("line", JSONValue.makeStringVal(line));
		JSONValue msg = new JSONValue();
		msg.mapVal = map;
		publish("logLine", JSONWriter.writeJSONValue(msg));
	}

	private void publish(String topic, String json) {
		if (pubSocket == null || !running)
			return;
		synchronized (pubSocket) {
			pubSocket.sendMore(topic.getBytes(ZMQ.CHARSET));
			pubSocket.send(json.getBytes(ZMQ.CHARSET));
		}
	}

	private void pollerLoop() {
		while (running) {
			try {
				Thread.sleep(ZmqConfig.SNAPSHOT_INTERVAL_MS);
			} catch (InterruptedException e) {
				break;
			}
			if (!running)
				break;
			try {
				publishSnapshot();
				publishEntityStates();
			} catch (Exception e) {
				Log.format("ZmqBridge snapshot error: %s", e.getMessage());
			}
		}
	}

	private void publishSnapshot() {
		if (simModel == null || pubSocket == null)
			return;

		double simTime = simModel.getSimTime();

		HashMap<String, JSONValue> root = new HashMap<>();
		root.put("type", JSONValue.makeStringVal("simState"));
		root.put("simTime", JSONValue.makeNumVal(simTime));

		String runState;
		if (!simModel.isStarted()) {
			runState = "IDLE";
		} else if (simModel.isEnded()) {
			runState = "ENDED";
		} else {
			runState = "RUNNING";
		}
		root.put("runState", JSONValue.makeStringVal(runState));

		ArrayList<JSONValue> entities = new ArrayList<>();
		for (Entity ent : simModel.getClonesOfIterator(Entity.class)) {
			if (!ent.isActive())
				continue;
			HashMap<String, JSONValue> ej = entityToJson(ent, simTime);
			entities.add(toJsonValue(ej));
		}
		root.put("entities", toJsonArray(entities));

		JSONValue msg = new JSONValue();
		msg.mapVal = root;
		publish("simState", JSONWriter.writeJSONValue(msg));
	}

	private void publishEntityStates() {
		if (simModel == null || pubSocket == null)
			return;

		double simTime = simModel.getSimTime();
		for (Entity ent : simModel.getClonesOfIterator(Entity.class)) {
			if (!ent.isActive())
				continue;
			HashMap<String, JSONValue> ej = entityToJson(ent, simTime);
			ej.put("show", JSONValue.makeStringVal(
				ent instanceof DisplayEntity ?
				Boolean.toString(((DisplayEntity) ent).getShow(simTime)) : "true"));
			JSONValue msg = new JSONValue();
			msg.mapVal = ej;
			publish("entityState", JSONWriter.writeJSONValue(msg));
		}
	}

	private HashMap<String, JSONValue> entityToJson(Entity ent, double simTime) {
		HashMap<String, JSONValue> e = new HashMap<>();
		e.put("name", JSONValue.makeStringVal(ent.getName()));
		e.put("type", JSONValue.makeStringVal(ent.getClass().getSimpleName()));

		if (ent instanceof DisplayEntity) {
			DisplayEntity de = (DisplayEntity) ent;
			e.put("position", vec3dToJson(de.getPosition()));
			e.put("size", vec3dToJson(de.getSize()));
		}

		if (ent instanceof StateEntity) {
			String state = ((StateEntity) ent).getPresentState(simTime);
			if (state != null) {
				e.put("state", JSONValue.makeStringVal(state));
			}
		}

		HashMap<String, JSONValue> outputs = new HashMap<>();
		for (ValueHandle vh : ent.getAllOutputs()) {
			JSONValue val = outputToJson(vh, simTime);
			if (val != null) {
				outputs.put(vh.getName(), val);
			}
		}
		e.put("outputs", toJsonValue(outputs));

		return e;
	}

	private JSONValue outputToJson(ValueHandle vh, double simTime) {
		try {
			double val = vh.getValueAsDouble(simTime, Double.NaN);
			if (!Double.isNaN(val) && !Double.isInfinite(val)) {
				return JSONValue.makeNumVal(val);
			}
		} catch (Exception e) {}
		try {
			String val = vh.getValue(simTime, String.class);
			if (val != null) {
				return JSONValue.makeStringVal(val);
			}
		} catch (Exception e) {}
		return null;
	}

	private JSONValue vec3dToJson(Vec3d v) {
		ArrayList<JSONValue> arr = new ArrayList<>();
		arr.add(JSONValue.makeNumVal(v.x));
		arr.add(JSONValue.makeNumVal(v.y));
		arr.add(JSONValue.makeNumVal(v.z));
		return toJsonArray(arr);
	}

	private JSONValue toJsonValue(HashMap<String, JSONValue> map) {
		JSONValue v = new JSONValue();
		v.mapVal = map;
		return v;
	}

	private JSONValue toJsonArray(ArrayList<JSONValue> list) {
		JSONValue v = new JSONValue();
		v.listVal = list;
		return v;
	}

	private void commandLoop() {
		while (running) {
			try {
				byte[] routingId = routerSocket.recv(0);
				if (routingId == null)
					continue;
				routerSocket.recv(0); // empty delimiter
				byte[] cmdBytes = routerSocket.recv(0);
				if (cmdBytes == null)
					continue;
				String cmd = new String(cmdBytes, ZMQ.CHARSET);
				String response = processCommand(cmd);

				routerSocket.sendMore(routingId);
				routerSocket.sendMore(new byte[0]);
				routerSocket.send(response.getBytes(ZMQ.CHARSET));
			} catch (Exception e) {
				if (running) {
					Log.format("ZmqBridge command error: %s", e.getMessage());
				}
			}
		}
	}

	private String processCommand(String cmdJson) {
		String cmd = extractStringField(cmdJson, "cmd");
		if (cmd == null) {
			return "{\"status\":\"error\",\"msg\":\"missing cmd field\"}";
		}

		switch (cmd.toUpperCase()) {
		case "PAUSE":
			return handlePause();

		case "SET_INPUT": {
			String entity = extractStringField(cmdJson, "entity");
			String keyword = extractStringField(cmdJson, "keyword");
			String value = extractStringField(cmdJson, "value");
			if (entity == null || keyword == null || value == null) {
				return "{\"status\":\"error\",\"msg\":\"SET_INPUT requires entity, keyword, value\"}";
			}
			return handleSetInput(entity, keyword, value);
		}

		default:
			return "{\"status\":\"error\",\"msg\":\"unknown command: " + cmd + "\"}";
		}
	}

	private String handlePause() {
		if (simModel == null)
			return "{\"status\":\"error\",\"msg\":\"no model\"}";
		if (simModel.getGUIListener() != null) {
			simModel.getGUIListener().pauseSimulation();
		}
		return "{\"status\":\"ok\"}";
	}

	private String handleSetInput(String entityName, String keyword, String value) {
		if (simModel == null)
			return "{\"status\":\"error\",\"msg\":\"no model\"}";
		Entity ent = simModel.getNamedEntity(entityName);
		if (ent == null) {
			return "{\"status\":\"error\",\"msg\":\"entity not found: " + entityName + "\"}";
		}
		try {
			InputAgent.applyArgs(ent, keyword, value);
			return "{\"status\":\"ok\"}";
		} catch (Exception e) {
			String msg = e.getMessage() != null ? e.getMessage() : e.getClass().getSimpleName();
			return "{\"status\":\"error\",\"msg\":\"" + msg + "\"}";
		}
	}

	private static String extractStringField(String json, String field) {
		String key = "\"" + field + "\"";
		int keyIdx = json.indexOf(key);
		if (keyIdx < 0)
			return null;

		int colonIdx = json.indexOf(':', keyIdx + key.length());
		if (colonIdx < 0)
			return null;

		int valStart = json.indexOf('"', colonIdx + 1);
		if (valStart < 0)
			return null;

		int valEnd = json.indexOf('"', valStart + 1);
		if (valEnd < 0)
			return null;

		return json.substring(valStart + 1, valEnd);
	}

}
