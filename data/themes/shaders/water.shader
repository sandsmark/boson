<vertex>

uniform vec3 lightPos;
uniform bool fogEnabled;

varying vec3 vertex;
varying vec3 lightDir;
varying float fogStrength;

void main()
{
  // Stuff that will be sent to fragment shader
  vertex = gl_Vertex.xyz;
  lightDir = normalize(lightPos);

  // Texture coordinate for accessing diffuse and normalmap textures
  gl_TexCoord[0] = gl_TextureMatrix[0] * vec4(gl_Vertex.xyz * 0.1, 1.0);

  gl_Position = ftransform();

  if(fogEnabled)
    fogStrength = clamp((gl_Position.z - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0);
}


<fragment>

// Diffuse map
uniform sampler2D texture_0;
// Bumpmap
uniform sampler2D texture_1;
// Envmap
uniform samplerCube texture_2;
uniform bool fogEnabled;

uniform vec3 cameraPos;
varying vec3 vertex;
varying vec3 lightDir;
varying float fogStrength;

// Index of refraction of water
#define IOR 1.333

void main()
{
  // Get normal from the normalmap
  vec3 normal = normalize(texture2D(texture_1, gl_TexCoord[0].xy).rgb * 2.0 - 1.0);

  // Diffuse light received by surface (N.L)
  vec3 diffuse = dot(lightDir, normal) * gl_LightSource[0].diffuse.rgb;

  // View-vector
  vec3 viewv = normalize(cameraPos - vertex);
  // Half-vector
  vec3 halfv = normalize(viewv + lightDir);
  // Amount of specular light
  float specular = pow(dot(halfv, normal), 80.0);
  // Specular color
  vec4 speccolor = gl_LightSource[0].specular * specular;

  // Color from the diffuse texture
  vec3 basetexcolor = texture2D(texture_0, gl_TexCoord[0].xy).rgb;

  // Color of the lit water surface
  vec3 litcolor = basetexcolor * (diffuse + gl_LightSource[0].ambient.rgb);

  // Color from envmap
  vec3 envcolor = textureCube(texture_2, reflect(vec3(viewv.x, viewv.y, -viewv.z), normal)).rgb * gl_LightSource[0].ambient.rgb;

  // Compute fresnel reflection/refraction factor
  float d = IOR * dot(viewv, normal);
  float refrFactor = clamp(d*d + 1.0 - IOR*IOR, 0.0, 1.0);


  // Final result
  // This will be mix between reflection (from envmap) and basecolor
  // Specular color will be added to the result
  vec3 visiblecolor = mix(envcolor, litcolor, refrFactor);
  if(fogEnabled)
    visiblecolor = mix(visiblecolor, gl_Fog.color.rgb, fogStrength);

  gl_FragColor = vec4(visiblecolor, 1.0 - refrFactor * 0.75) + speccolor;
}

