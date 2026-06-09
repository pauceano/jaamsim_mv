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

import com.jaamsim.math.Vec3d;

public class EntityShadow {

	public String name;
	public String type;
	public Vec3d position = new Vec3d();
	public Vec3d size = new Vec3d(1.0d, 1.0d, 1.0d);
	public Vec3d orientation = new Vec3d();
	public boolean show = true;
	public String state;
	public ArrayList<String> displayModels = new ArrayList<>();
	public HashMap<String, Double> outputs = new HashMap<>();

	public EntityShadow() {}

	public EntityShadow(String name, String type) {
		this.name = name;
		this.type = type;
	}

	public void setPosition(Vec3d v) {
		position.x = v.x;
		position.y = v.y;
		position.z = v.z;
	}

	public void setSize(Vec3d v) {
		size.x = v.x;
		size.y = v.y;
		size.z = v.z;
	}

	public void setOrientation(Vec3d v) {
		orientation.x = v.x;
		orientation.y = v.y;
		orientation.z = v.z;
	}

}
