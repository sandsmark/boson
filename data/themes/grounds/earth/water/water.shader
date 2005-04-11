<vertex>

uniform vec3 lightPos;

varying vec3 vertex;
varying vec3 lightDir;

void main()
{
  // Stuff that will be sent to fragment shader
  vertex = gl_Vertex.xyz;
  lightDir = normalize(lightPos);

  gl_Position = ftransform();
}


<fragment>

// Diffuse map
uniform sampler2D texture_0;
// Bumpmap
uniform sampler2D texture_1;
// Envmap
uniform samplerCube texture_2;
uniform vec3 cameraPos;

varying vec3 vertex;
varying vec3 lightDir;

// Index of refraction of water
#define IOR 1.333

void main()
{
  // Texture coordinate for accessing diffuse and normalmap textures
  vec2 texcoord = gl_TextureMatrix[0] * vec4(vertex * 0.1, 1.0f);

  // Get normal from the normalmap
  vec3 normal = normalize(texture2D(texture_1, texcoord).xyz * 2.0 - 1.0);

  // Amount of diffuse light received by surface (N.L)
  float diffuse = dot(lightDir, normal) * 0.6;

  // View-vector
  vec3 viewv = normalize(cameraPos - vertex);
  // Half-vector
  vec3 halfv = normalize(viewv + lightDir);
  // Amount of specular light
  float specular = pow(dot(halfv, normal), 80);
  // Specular color
  vec4 speccolor = vec4(1.0, 0.95, 0.8, 1.0) * specular;

  // Color from the diffuse texture
  vec3 basecolor = texture2D(texture_0, vertex * 0.1).xyz;

  // Color from envmap
  vec3 envcolor = textureCube(texture_2, reflect(viewv, normal)).xyz;

  // Compute fresnel reflection/refraction factor
  float d = IOR * dot(viewv, normal);
  float refrFactor = clamp(d*d + 1.0 - IOR*IOR, 0.0, 1.0);

  // Final result
  // This will be mix between reflection (from envmap) and basecolor
  // Specular color will be added to the result
  gl_FragColor = vec4(mix(envcolor, basecolor * (diffuse + 0.4), refrFactor), 1.0 - refrFactor * 0.75) + speccolor;
}

