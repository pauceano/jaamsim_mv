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

import com.jaamsim.Graphics.DisplayEntity;
import com.jaamsim.math.Transform;
import com.jaamsim.math.Vec3d;

public class EntityShadowAdapter extends DisplayEntity {

	private final EntityShadow shadow;

	public EntityShadowAdapter(EntityShadow shadow) {
		this.shadow = shadow;
	}

	@Override
	public boolean getShow(double simTime) {
		return shadow.show;
	}

	@Override
	public Vec3d getPosition() {
		return shadow.position;
	}

	@Override
	public Vec3d getSize() {
		return shadow.size;
	}

	@Override
	public Vec3d getOrientation() {
		return shadow.orientation;
	}

	@Override
	public Vec3d getGlobalPosition() {
		return shadow.position;
	}

	@Override
	public Transform getGlobalTrans() {
		Transform ret = new Transform(shadow.position);
		ret.setScale(shadow.size.x);
		return ret;
	}

	@Override
	public long getEntityNumber() {
		return shadow.name.hashCode() & 0x7FFFFFFFL;
	}

}
