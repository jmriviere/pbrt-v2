/*
 * ray_caster.cl
 *
 *  Created on: Jun 21, 2013
 *      Author: poupine
 */

#include "GPU.h"

float3 transform(float3 r, Transformation t) {
	float4 homogeneous = (float4)(r, 1) ;

	float3 ret;

	homogeneous.s3 = dot(t.m[3], homogeneous);
	ret.s0 = dot(t.m[0], homogeneous)/homogeneous.s3;
	ret.s1 = dot(t.m[1], homogeneous)/homogeneous.s3;
	ret.s2 = dot(t.m[2], homogeneous)/homogeneous.s3;

	return ret;
}

Hit ray_sphere_intersection(Ray ray, Sphere sphere, Metadata meta_sphere) {

	Hit h;

	Ray rhit;

	rhit.origin = transform(ray.origin, meta_sphere.fromWorld);
	rhit.direction = transform(ray.direction, meta_sphere.fromWorld);

	float A = dot(ray.direction, ray.direction);
	float B = 2 * dot(ray.direction, ray.origin);
	float C = dot(ray.origin, ray.origin) - sphere.radius * sphere.radius;

	float inv2A = 1/(2 * A);

	float delta = B*B - 4*A*C;

	h.ray = rhit;

	if (delta < 0) {
		h.t = -1;
	}
	else {
		float sqrt_d = sqrt(delta);
		float t1 = (-B - sqrt_d) * inv2A;
		float t2 = (-B + sqrt_d) * inv2A;

		h.t =  (t1 > t2 ? t1 : t2);
	}
	
	return h;
}

Hit intersection(Ray ray, __global float* prims, Metadata meta_prim) {
	switch (meta_prim.type) {
	case sphere:
		Sphere s;
		s.radius = *(prims + meta_prim.offset);
		return ray_sphere_intersection(ray, s, meta_prim);
		break;
	default:
		break;
	}
}

Ray reflection(Hit h) {
	float3 n = normalize(h.ray.origin + h.t * h.ray.direction);
	Ray ref;
	ref.direction = 2 * dot(h.ray.direction, n) * n - h.ray.direction;
	ref.origin = n;
	return ref;
}

Color lookup(image2d_t env, Metadata meta_env, Ray ray) {
	float3 dir = normalize(transform(ray.direction, meta_env.fromWorld));
	float theta = acos(dir.s2);
	float phi = atan2(dir.s1, dir.s0);
	float x = (phi < 0 ? phi + 2 * M_PI : phi)/(2 * M_PI); //(phi + M_PI)/(2 * M_PI);
	float y = theta/M_PI;
	Color c = read_imagef(env, sampler, (float2)(x,y));
	return c;
}


__kernel void ray_cast(__global float4* Ls, __global Ray* rays, int nb_prim, __global Metadata* meta_prims,
					   __global float* prims, __global Metadata* meta_light, __read_only image2d_t env) {

	// Index
	int p = get_global_id(0);


	Hit hit;
	for (int i = 0; i < nb_prim; ++i) {
		hit = intersection(rays[p], prims, meta_prims[i]);
		if (hit.t != -1) {
			Color c = lookup(env, meta_light[0], reflection(hit));
			Ls[p] = c;
		}
		else {
		     Color c = lookup(env, meta_light[0], rays[p]);
		     Ls[p] = c;
		}
	}
}
