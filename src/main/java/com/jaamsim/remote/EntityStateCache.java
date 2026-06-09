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
import java.util.concurrent.ConcurrentHashMap;

import org.zeromq.SocketType;
import org.zeromq.ZContext;
import org.zeromq.ZMQ;

import com.jaamsim.basicsim.Log;

public class EntityStateCache {

	private final ConcurrentHashMap<String, EntityShadow> entities = new ConcurrentHashMap<>();
	private final ZContext context;
	private final ZMQ.Socket subSocket;
	private Thread subscriberThread;
	private volatile boolean running;

	public EntityStateCache() {
		context = new ZContext();
		subSocket = context.createSocket(SocketType.SUB);
	}

	public void start() {
		running = true;
		subSocket.connect("tcp://localhost:5556");
		subSocket.subscribe("entityState".getBytes(ZMQ.CHARSET));

		subscriberThread = new Thread(this::subscriberLoop, "EntityStateSubscriber");
		subscriberThread.setDaemon(true);
		subscriberThread.start();
	}

	public void stop() {
		running = false;
		if (subscriberThread != null) {
			try { subscriberThread.join(2000); } catch (InterruptedException e) {}
		}
		context.close();
	}

	public EntityShadow getShadow(String name) {
		return entities.get(name);
	}

	public ArrayList<EntityShadow> getSnapshots() {
		return new ArrayList<>(entities.values());
	}

	public int size() {
		return entities.size();
	}

	private void subscriberLoop() {
		while (running) {
			try {
				byte[] topicBytes = subSocket.recv(0);
				if (topicBytes == null)
					continue;
				byte[] jsonBytes = subSocket.recv(0);
				if (jsonBytes == null)
					continue;
				parseAndStore(new String(jsonBytes, ZMQ.CHARSET));
			} catch (Exception e) {
				if (running) {
					Log.format("EntityStateCache error: %s", e.getMessage());
				}
			}
		}
	}

	private void parseAndStore(String json) {
		EntityShadow shadow = new EntityShadow();

		shadow.name = extractString(json, "name");
		shadow.type = extractString(json, "type");
		if (shadow.name == null)
			return;

		String showStr = extractString(json, "show");
		shadow.show = !"false".equals(showStr);

		shadow.position = extractVec3d(json, "position");
		shadow.size = extractVec3d(json, "size");
		shadow.orientation = extractVec3d(json, "orientation");
		shadow.state = extractString(json, "state");

		entities.put(shadow.name, shadow);
	}

	private static String extractString(String json, String key) {
		String search = "\"" + key + "\"";
		int keyIdx = json.indexOf(search);
		if (keyIdx < 0)
			return null;

		int colonIdx = json.indexOf(':', keyIdx + search.length());
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

	private static com.jaamsim.math.Vec3d extractVec3d(String json, String key) {
		String search = "\"" + key + "\"";
		int keyIdx = json.indexOf(search);
		if (keyIdx < 0)
			return new com.jaamsim.math.Vec3d();

		int bracketIdx = json.indexOf('[', keyIdx);
		if (bracketIdx < 0)
			return new com.jaamsim.math.Vec3d();

		int endIdx = json.indexOf(']', bracketIdx);
		if (endIdx < 0)
			return new com.jaamsim.math.Vec3d();

		String arr = json.substring(bracketIdx + 1, endIdx);
		String[] parts = arr.split(",");
		if (parts.length < 3)
			return new com.jaamsim.math.Vec3d();

		try {
			return new com.jaamsim.math.Vec3d(
				Double.parseDouble(parts[0].trim()),
				Double.parseDouble(parts[1].trim()),
				Double.parseDouble(parts[2].trim())
			);
		} catch (NumberFormatException e) {
			return new com.jaamsim.math.Vec3d();
		}
	}

}
