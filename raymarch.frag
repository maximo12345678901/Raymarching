uniform vec2 uResolution;
uniform vec3 uCamPos;
uniform vec3 uForward;
uniform vec3 uRight;
uniform vec3 uUp;
uniform float uFov;
uniform float uBlendStrength;

uniform int Iterations;
uniform float Power;

float BoxSDF(vec3 p, vec3 b) {
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
    // return length(max(q,0.0)) + min(max(q.x* q.y,min(q.y*q.z,q.z*q.x)),0.0);
}

float TorusSDF(vec3 p, vec2 t) {
    vec2 q = vec2(length(p.xz) - t.x, p.y);
    return length(q) - t.y;
}

float BoxFrameSDF(vec3 p, vec3 b, float e) {
    p = abs(p) - b;
    vec3 q = abs(p + e) - e;
    return min(min(
        length(max(vec3(p.x, q.y, q.z), 0.0)) + min(max(p.x, max(q.y, q.z)), 0.0),
        length(max(vec3(q.x, p.y, q.z), 0.0)) + min(max(q.x, max(p.y, q.z)), 0.0)),
        length(max(vec3(q.x, q.y, p.z), 0.0)) + min(max(q.x, max(q.y, p.z)), 0.0));
}

float LinkSDF( vec3 p, float le, float r1, float r2 )
{
  vec3 q = vec3( p.x, max(abs(p.y)-le,0.0), p.z );
  return length(vec2(length(q.xy)-r1,q.z)) - r2;
}

float DE(vec3 pos) {
	vec3 z = pos;
	float dr = 1.0;
	float r = 0.0;
	for (int i = 0; i < Iterations ; i++) {
		r = length(z);
		if (r>4.0) break;
		
		// convert to polar coordinates
		float theta = acos(z.z/r);
		float phi = atan(z.y,z.x);
		dr =  pow( r, Power-1.0)*Power*dr + 1.0;
		
		// scale and rotate the point
		float zr = pow( r,Power);
		theta = theta*Power;
		phi = phi*Power;
		
		// convert back to cartesian coordinates
		z = zr*vec3(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
		z+=pos;
	}
	return 0.5*log(r)*r/dr;
}

float pmod(float a, float b) {
    return mod(mod(a, b) + b, b);
}

float SmoothMin(float distA, float distB, float k) {
    float h = max(k-abs(distA-distB), 0.0) / k;
    return min(distA, distB) - h*h*h*k*1.0/6.0;
}

vec3 BendDir(vec3 dir, float k) {
    float c = cos(k), s = sin(k);
    return normalize(vec3(c*dir.x - s*dir.y, s*dir.x + c*dir.y, dir.z));
}
vec3 BendPos(vec3 pos, float k) {
    float c = cos(k * pos.x), s = sin(k * pos.x);
    return vec3(pos.x, c*pos.y - s*pos.z, s*pos.y + c*pos.z);
}

float Scene(vec3 p) {
    // float dTorus    = TorusSDF(p, vec2(2.0, 1.0));
    // float dBoxFrame = BoxFrameSDF(p - vec3(5.0, 1.5, 5.0), vec3(2.0, 1.0, 2.5), 0.1);
    // float dLink = LinkSDF(vec3(p.x, pmod(p.y, 2.0), p.z), 2.0, 1.0, 0.5);
    // return SmoothMin(SmoothMin(dTorus, dBoxFrame, uBlendStrength), dLink, uBlendStrength);
    return DE(p);
}

vec3 Normal(vec3 p) {
    float e = 0.001;
    return normalize(vec3(
        Scene(vec3(p.x+e, p.y, p.z)) - Scene(vec3(p.x-e, p.y, p.z)),
        Scene(vec3(p.x, p.y+e, p.z)) - Scene(vec3(p.x, p.y-e, p.z)),
        Scene(vec3(p.x, p.y, p.z+e)) - Scene(vec3(p.x, p.y, p.z-e))
    ));
}

void main() {
    vec2 uv = (gl_FragCoord.xy / uResolution) * 2.0 - 1.0;
    float aspectRatio = uResolution.x / uResolution.y;

    float halfFovTan = tan(uFov / 2.0);

    vec3 rayDir = normalize(
        uForward
        + uRight * (uv.x * halfFovTan * aspectRatio)
        + uUp    * (uv.y * halfFovTan)
    );

    vec3 rayPos = uCamPos;
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));

    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

    // vec3 warpedRayDir = rayDir;

    for (int i = 0; i < 50; i++) {
        float dist = Scene(rayPos);
        // warpedRayDir = BendDir(warpedRayDir, 0.2);
        rayPos += rayDir * dist;

        if (dist < 0.01) {
            vec3 normal = Normal(rayPos);
            float brightness = (1.0 + dot(lightDir, normal)) / 2.0;

            vec3 shadowPos = rayPos + normal * 0.05;
            bool inShadow = false;
            for (int j = 0; j < 300; j++) {
                float shadowDist = Scene(shadowPos);
                shadowPos += lightDir * shadowDist;
                if (shadowDist < 0.01) {
                    inShadow = true;
                    break;
                }
            }

            if (inShadow)
                brightness *= 0.4;

            color = vec4(brightness, brightness, brightness, 1.0);
            break;
        }

        if (dist > 100.0) {
            if (dot(rayDir, lightDir) > 0.999)
                color = vec4(1.0, 1.0, 0.0, 1.0);
            break;
        }
    }

    gl_FragColor = color;
}
