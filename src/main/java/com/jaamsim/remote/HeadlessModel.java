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

import java.io.File;

import com.jaamsim.basicsim.JaamSimModel;
import com.jaamsim.basicsim.RunListener;

public class HeadlessModel {

	public static void main(String[] args) {
		if (args.length < 1) {
			System.err.println("Usage: HeadlessModel <config-file>");
			System.exit(1);
		}

		File configFile = new File(args[0]);
		if (!configFile.exists()) {
			System.err.println("Config file not found: " + args[0]);
			System.exit(1);
		}

		JaamSimModel simModel = new JaamSimModel();
		simModel.autoLoad();

		try {
			simModel.configure(configFile);
		} catch (Exception e) {
			System.err.println("Configure warnings: " + e.getMessage());
		}

		ZmqBridge bridge = new ZmqBridge();
		bridge.start(simModel);

		System.out.println("ZMQ bridge started on tcp://*:5556 (PUB) and tcp://*:5557 (ROUTER)");
		System.out.println("Running simulation...");

		final Object lock = new Object();
		simModel.start(new RunListener() {
			@Override
			public void runEnded() {
				synchronized (lock) { lock.notifyAll(); }
			}

			@Override
			public void handleRuntimeError(JaamSimModel sm, Throwable t) {
				System.err.println("Runtime error: " + t.getMessage());
				t.printStackTrace();
				synchronized (lock) { lock.notifyAll(); }
			}
		});

		synchronized (lock) {
			try { lock.wait(); } catch (InterruptedException e) {}
		}

		System.out.println("Simulation ended. Sim time: " + simModel.getSimTime() + " s");
		bridge.stop();
		System.exit(0);
	}

}
