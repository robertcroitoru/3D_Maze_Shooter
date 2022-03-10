#version 330

// Input
in vec3 world_position;
in vec3 world_normal;

// Uniforms for light properties
uniform vec3 light_direction;
uniform vec3 light_position;
uniform vec3 eye_position;
uniform mat3 bullet_lights;

uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;

// TODO(student): Declare any other uniforms

uniform vec3 object_color;
uniform int spot;
uniform float angle;

// Output
layout(location = 0) out vec4 out_color;


void main()
{


	vec3 totalLight = vec3(0);
	for (int i = 0; i < 3; i++) {
		float intensitateLumina;
		float primesteLumina;
		vec3 N = normalize(world_normal);
		vec3 L = normalize(bullet_lights[i] - world_position);
		vec3 V = normalize(eye_position - world_position);
		vec3 H = normalize(L + V);
		vec3 R = normalize(reflect(-L, N));

		// TODO(student): Define ambient, diffuse and specular light components
		float ambient_light = material_kd * 0.25;
		if (i > 0) ambient_light = 0;
		float diffuse_light = material_kd * max(dot(normalize(N), L), 0);

		if (dot(N, L) > 0)
			primesteLumina = 1;
		else
			primesteLumina = 0;
		float specular_light = 0;
		//specular_light = material_ks * intensitateLumina * primesteLumina * pow(max(dot(V, R), 0), material_shininess);



		// It's important to distinguish between "reflection model" and
		// "shading method". In this shader, we are experimenting with the Phong
		// (1975) and Blinn-Phong (1977) reflection models, and we are using the
		// Phong (1975) shading method. Don't mix them up!
		if (diffuse_light > 0)
		{
			specular_light = material_ks * pow(max(dot(normalize(N), H), 0), material_shininess);
		}

		//   float dist = distance(light_position, world_position);
		   //float attenuationFactor = 1 / (1 + 0.01 * dist + 0.05 * dist * dist);
		   //float attenuationFactor = 1 / pow(length(light_position - world_position), 2);


		   // TODO(student): If (and only if) the light is a spotlight, we need to do
		   // some additional things.
		float cut_off = angle;
		float spot_light = dot(-L, light_direction);
		float spot_light_limit = cos(cut_off);
		// Quadratic attenuation
		float linear_att = (spot_light - spot_light_limit) / (1 - spot_light_limit);
		float light_att_factor = pow(linear_att, 2);
		float attenuationFactor = 1 / pow(length(bullet_lights[i] - world_position), 2);

		vec3 light;

		if (spot == 0) {
			light = object_color * (ambient_light + attenuationFactor * (diffuse_light + specular_light));
		}
		else {

			if (spot_light > cos(cut_off))
			{

				light = object_color * (ambient_light + (light_att_factor + 0.01 * attenuationFactor) * (diffuse_light + specular_light));
			}
			else {
				light = ambient_light * object_color;
			}
		}

		totalLight = totalLight + light;
	}



	out_color = vec4(totalLight, 1);

}
