/*
 * ray_caster.cl
 *
 *  Created on: Jun 21, 2013
 *      Author: poupine
 */

#include "GPU.h"

float3 transform(float3 r, Transformation t) {
	float3 ret;

	ret.s0 = dot(t.m[0], (float4)(r, 1));
	ret.s1 = dot(t.m[1], (float4)(r, 1));
	ret.s2 = dot(t.m[2], (float4)(r, 1));

	return ret;
}


Ray reflection(Ray ray, float3 n) {
	Ray ref;
	ref.direction = 2 * dot(ray.direction, n) * n - ray.direction;
	ref.origin = n;
	return ref;
}

#define OFFSETOF(type, field)    ((unsigned long) &(((type *) 0)->field))

bool ray_sphere_intersection(Hit* hit, Ray ray, Metadata m_sphere, __global float* prims) {

	Sphere sphere;
	sphere.radius = prims[m_sphere.offset];

	Ray trans = ray;
	trans.origin = transform(ray.origin, m_sphere.fromWorld);

	float A = dot(trans.direction, trans.direction);
	float B = 2 * dot(trans.direction, trans.origin);
	float C = dot(trans.origin, trans.origin) - sphere.radius * sphere.radius;

	float inv2A = 1/(2 * A);

	float delta = B*B - 4*A*C;

	if (delta < 0) {
		return false;
	}
	float sqrt_d = sqrt(delta);
	float t1 = (-B - sqrt_d) * inv2A;
	float t2 = (-B + sqrt_d) * inv2A;

	hit->t =  (t1 > t2 ? t1 : t2);

	return true;
}

//A revoir
bool intersect(Hit* hit, Ray ray, __global Metadata* meta_prims, __global float* prims, int nb_prims) {
	bool intersect = false;

	hit->t = 1e5;

	for (uint i = 0; i < nb_prims; ++i) {
		switch (meta_prims[i].type) {
		case sphere:
			float temp = hit->t;
			if (ray_sphere_intersection(hit, ray, meta_prims[i], prims) && hit->t < temp) {
				hit->id = i;
			}
			else {
				hit->t = temp;
			}
			break;
		default:
			break;
		}
	}
	return intersect;
}


Color lookup(image2d_t env, Metadata meta_env, Ray ray) {
	//float3 dir = normalize(transform(ray.direction, meta_env.fromWorld));
	float3 dir = ray.direction;
	float theta = acos(dir.s2);
	float phi = atan2(dir.s1, dir.s0);
	float x = (phi < 0 ? phi + 2 * M_PI : phi)/(2 * M_PI); //(phi + M_PI)/(2 * M_PI);
	float y = theta/M_PI;
	Color c = read_imagef(env, sampler, (float2)(x,y));
	return c;
}

Color radiance(image2d_t env, Ray ray, __global Metadata* meta_prims, __global float* prims, int nb_prims) {

	Hit hit;

	int i = 0;

	while(intersect(&hit, ray, meta_prims, prims, nb_prims) && i++ <= 3) {
		ray.origin = transform(ray.origin, meta_prims[hit.id].fromWorld);
		float3 n = normalize(ray.origin + hit.t * ray.direction);
		ray.origin = transform(ray.origin, meta_prims[hit.id].toWorld);
		ray = reflection(ray, n);
	}
	return lookup(env, meta_prims[hit.id], ray);
}

__kernel void ray_cast(__global float4* Ls, __global Ray* rays, int nb_prims, __global Metadata* meta_prims,
					   __global float* prims, __global Metadata* meta_light, __read_only image2d_t env) {

	// Index
	int p = get_global_id(0);

	Hit hit;

	Ls[p] = radiance(env, rays[p], meta_prims, prims, nb_prims);
}
