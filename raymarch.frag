uniform vec2 uResolution;
uniform vec3 uCamPos;
uniform vec3 uForward;
uniform vec3 uRight;
uniform vec3 uUp;
uniform float uFov;

float BoxSDF(vec3 p, vec3 b) {
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(min(q.x, max(q.y, q.z)), 0.0);
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
        length(max(vec3(q.x, q.y, p.z), 0.0)) + min(max(q.x, max(q.y, p.z)), 0.0)) - 0.1;
}

float Scene(vec3 p) {
    float dDiamond  = BoxSDF(p, vec3(1.0, 2.0, 1.0));
    float dTorus    = TorusSDF(p - vec3(1.0, 1.0, 0.0), vec2(1.0, 0.5));
    float dBoxFrame = BoxFrameSDF(p - vec3(0.0, 2.0, 0.0), vec3(2.0, 1.0, 2.5), 0.1);
    return min(max(dDiamond, -dTorus), dBoxFrame);
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

    for (int i = 0; i < 100; i++) {
        float dist = Scene(rayPos);
        rayPos += rayDir * dist;

        if (dist < 0.01) {
            vec3 normal = Normal(rayPos);
            float brightness = (1.0 + dot(lightDir, normal)) / 2.0;

            vec3 shadowPos = rayPos + normal * 0.05;
            bool inShadow = false;
            for (int j = 0; j < 30; j++) {
                float shadowDist = Scene(shadowPos);
                shadowPos += lightDir * shadowDist;
                if (shadowDist < 0.01) {
                    inShadow = true;
                    break;
                }
            }

            if (inShadow)
                brightness *= 0.1;

            color = vec4(brightness, brightness, brightness, 1.0);
            break;
        }

        if (dist > 50.0) {
            if (dot(rayDir, lightDir) > 0.999)
                color = vec4(1.0, 1.0, 0.0, 1.0);
            break;
        }
    }

    gl_FragColor = color;
}
