/*
 * ray_caster.cl
 *
 *  Created on: Jun 21, 2013
 *      Author: poupine
 */

#include "GPU.h"
#include "ray_caster.h"
#include "fresnel.h"

bool ray_sphere_intersection(Hit* hit, Ray ray, Metadata m_sphere, __global float* prims) {

	Sphere sphere;
	sphere.radius = prims[m_sphere.offset];

	Ray trans = ray;
	trans.origin = transform_point(ray.origin, m_sphere.fromWorld);
	trans.direction = transform_vect(ray.direction, m_sphere.fromWorld);

	float A = dot(trans.direction, trans.direction);
	float B = 2 * dot(trans.direction, trans.origin);
	float C = dot(trans.origin, trans.origin) - sphere.radius * sphere.radius;

	float delta = B*B - 4*A*C;

	if (delta < 0) {
		return false;
	}

	float sqrt_d = sqrt(delta);

	float q;

	if (B < 0)
	        q = -0.5 * (B - sqrt_d);
	    else
	        q = -0.5 * (B + sqrt_d)/2.0;

	float t0 = q/A;
	float t1 = C/q;

	// make sure t0 is smaller than t1
	if (t0 > t1)
	{
		// if t0 is bigger than t1 swap them around
		float temp = t0;
		t0 = t1;
		t1 = temp;
	}

	// if t1 is less than zero, the object is in the ray's negative direction
	// and consequently the ray misses the sphere
	if (t1 < 0)
		return false;

	// if t0 is less than zero, the intersection point is at t1
	if (t0 < 0)
	{
		hit->t = t1;
	}
	// else the intersection point is at t0
	else
	{
		hit->t = t0;
	}

	return true;
}

//A revoir
bool intersect(Hit* hit, Ray ray, __global Metadata* meta_prims, __global float* prims, int nb_prims) {
	float temp;
	hit->t = 1e5;

	for (uint i = 0; i < nb_prims; ++i) {
		switch (meta_prims[i].type) {
		case sphere:
			temp = hit->t;
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
	return hit->t < 1e5;
}


Color lookup(image2d_t env, Metadata meta_env, Ray ray) {
	float3 dir = normalize(ray.direction);
	dir.s2 = (dir.s2 > 1.f ? 1.f : (dir.s2 < -1.f ? -1.f : dir.s2));
	float theta = acos(dir.s2);
	float phi = atan2(dir.s1, dir.s0);
	float x = (phi < 0.f ? phi + 2 * M_PI : phi)/(2 * M_PI);
	float y = theta/M_PI;
	Color c = read_imagef(env, sampler, (float2)(x,y));
	return c;
}

Color radiance(image2d_t env, Ray ray, __global Metadata* meta_prims, __global float* prims,
			   int nb_prims) {

	Color reflectance = (Color)(1, 1, 1, 1);
	Color cl = (Color)(0,0,0,0);

	threefry4x32_key_t k = {{get_global_id(0), 0xdecafbad, 0xfacebead, 0x12345678}};
	threefry4x32_ctr_t cc = {{0, 0xf00dcafe, 0xdeadbeef, 0xbeeff00d}};
	threefry4x32_ctr_t r;

	Hit hit;

	uint i = 0;
	float3 hitpoint;

	while(true) {
		cc.v[0]++;
		r = threefry4x32(cc, k);

		float rand = u01_open_open_32_24(r.v[0]);

		if (!intersect(&hit, ray, meta_prims, prims, nb_prims)) {
			cl += reflectance * lookup(env, meta_prims[hit.id], ray);
			return cl;
		}

		float p = min(0.5f, reflectance.s1);

		if (i++ > 5) {
			if (rand < p) {
				reflectance /= p;
			}
			else {
				return cl;
			}
		}

		if (hit.t < 1e-2) {
			ray.origin += 1e-2 * ray.direction;
			continue;
		}

		hitpoint = ray.origin + hit.t * ray.direction; // hitpoint in world space
		float3 n = normalize(transform_point(hitpoint, meta_prims[hit.id].fromWorld));
		ray.origin = hitpoint;


		//switch (meta_prims[hit.id].mat) {
		switch (hit.id) {
		case 0:
			ray.direction = reflection(ray, n);
			break;
			/*case DIFF:
			//default:
				float x = cos(2.f * M_PI * rand) * sqrt(1 - rand * rand);
				float y = sin(2.f * M_PI * rand) * sqrt(1 - rand * rand);
				float z = rand;
				ray.direction = normalize((float3)(x, y, z));
				break;*/
		//case REFR:
		case 1:
			float3 nl = dot(ray.direction, n) < 0 ? n : -n;
			bool into = dot(n, nl) > 0;
			float3 reflectiondir = reflection(ray, nl);

			float nc=1.f, nt=1.5, nnt=into?nc/nt:nt/nc, ddn=dot(ray.direction, nl), cos2t;

			if ((cos2t=1-nnt*nnt*(1-ddn*ddn))<0){
				ray.direction = reflectiondir;
				continue;
			}

//			float3 transmissiondir = refraction(ray, nl, nnt);
			float3 transmissiondir = normalize(ray.direction*nnt - n*((into?1:-1)*(ddn*nnt+sqrt(cos2t))));
			float a= nt-nc, b=nt+nc, R0=a*a/(b*b), c = 1-(into?-ddn:dot(n, transmissiondir));
			float Re = R0 + (1-R0) * c * c * c * c * c; //Rpercent(ray, n, nc, nt);
			float Tr = 1.f-Re, P = 0.25 + 0.5 * Re, Rp = Re/P, Tp = Tr/(1-P);

			rand = u01_open_open_32_24(r.v[0]);

			if (rand < P) {
				reflectance *= Rp;
				ray.direction = reflectiondir;
			}
			else {
				reflectance *= Tp;
				ray.direction = transmissiondir;
			}
			break;
		default:
			float eps1 = u01_open_open_32_24(r.v[0]);
			float eps2 = u01_open_open_32_24(r.v[1]);

			float theta = acos(sqrt(1.0 - eps1));
			float phi = 2.0 * M_PI * eps2;

			float xs = sin(theta) * cos(phi);
			float ys = sin(theta) * sin(phi);
			float zs = cos(theta);

			float3 h = n;

			if (fabs(h.x) <= fabs(h.y) && fabs(h.x) <= fabs(h.z)) {
				h.x = 1.0;
			}
			else if (fabs(h.y) <= fabs(h.x) && fabs(h.y) <= fabs(h.z)) {
				h.y = 1.0;
			}
			else {
				h.z = 1.0;
			}

			float3 x = normalize(cross(h, n));
			float3 y = normalize(cross(x, n));

			ray.direction = (float3)normalize(xs * x + ys * y + zs * n);
			//reflectance *= M_PI / (cos(theta) * sin(theta));
			break;
		}
	}
}

__kernel void ray_cast(__global float4* Ls, __global Ray* rays, int nb_prims, __global Metadata* meta_prims,
					   __global float* prims, __global Metadata* meta_light, __read_only image2d_t env) {

	// Index
	int p = get_global_id(0);

	Hit hit;

	Ls[p] = radiance(env, rays[p], meta_prims, prims, nb_prims);
}
