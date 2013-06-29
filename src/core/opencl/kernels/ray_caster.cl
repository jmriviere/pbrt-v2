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

Hit ray_sphere_intersection(Ray ray, Sphere sphere) {

	Hit h;

	Ray rhit;

	rhit.origin = transform(ray.origin, sphere.w2o);
	rhit.direction = ray.direction;

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

Ray reflection(Hit h) {
	float3 n = normalize(h.ray.origin + h.t * h.ray.direction);
	Ray ref;
	ref.direction = 2 * dot(h.ray.direction, n) * n - h.ray.direction;
	ref.origin = n;
	return ref;
}

Color lookup(image2d_t env, Ray ray) {
	float3 dir = normalize(transform(ray.direction, w2l));
	float theta = acos(dir.s2);
	float phi = atan2(dir.s1, dir.s0);
	float x = (phi < 0 ? phi + 2 * M_PI : phi)/(2 * M_PI); //(phi + M_PI)/(2 * M_PI);
	float y = theta/M_PI;
	Color c = read_imagef(env, sampler, (float2)(x,y));
	return c;
}

__kernel void ray_cast(__read_only image2d_t env, __global float4* Ls, __global Ray* rays, int nb_prim, __global Sphere* spheres) {

	int p = get_global_id(0);
	Hit hit;
	for (int i = 0; i < nb_prim; ++i) {
		if (0 == p) {
			printf("Lololol %d\n", sizeof(Sphere));
			printf("Radius: %f", spheres[i].radius);
		}
		hit = ray_sphere_intersection(rays[p], spheres[i]);
		if (hit.t != -1) {
			Color c = lookup(env, reflection(hit));
			Ls[p] = c;
		}
		else {
		     Color c = lookup(env, rays[p]);
		     Ls[p] = c;
		}
	}
	if(p == 0) {
		for(int i =0; i < 4; ++i){
			printf("\n%f,%f,%f,%f\n", w2l.m[i].s0,w2l.m[i].s1,w2l.m[i].s2,w2l.m[i].s3);
		}
	}
}
