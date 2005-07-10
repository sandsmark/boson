<vertex>


uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform bool fogEnabled;

varying float alpha;
varying vec3 tLightDir;
varying vec3 tCameraDir;

void main()
{
  // Stuff that will be sent to fragment shader
  alpha = gl_Color.w;

  // Texture coordinate for accessing diffuse and normalmap textures
  gl_TexCoord[0] = gl_TextureMatrix[0] * vec4(gl_Vertex.xyz, 1.0);

  // Texture coordinate for accessing fog texture
  gl_TexCoord[1] = gl_TextureMatrix[1] * vec4(gl_Vertex.xyz, 1.0);

  // Calculate tangent and binormal
  vec3 tangent = normalize(vec3(gl_Normal.z, 0.0, -gl_Normal.x));
  vec3 binormal = normalize(vec3(0.0, gl_Normal.z, -gl_Normal.y));
  // Calculate tangent-space light- and view-vectors
  vec3 l = lightPos - gl_Vertex.xyz;
  vec3 v = cameraPos - gl_Vertex.xyz;
  tLightDir = normalize(vec3(dot(l, tangent),  dot(l, binormal),  dot(l, gl_Normal.xyz)));
  tCameraDir = normalize(vec3(dot(v, tangent),  dot(v, binormal),  dot(v, gl_Normal.xyz)));

  gl_Position = ftransform();

  if(fogEnabled)
    gl_FogFragCoord = gl_Position.z;
}


<fragment>

// Diffuse map
uniform sampler2D texture_0;
// Fog map
uniform sampler2D texture_1;
// Bumpmap
uniform sampler2D texture_2;

uniform float bumpScale;
uniform float bumpBias;
uniform bool fogEnabled;

varying float alpha;
varying vec3 tLightDir;
varying vec3 tCameraDir;

void main()
{
  // Don't render invisible terrain
  if(alpha == 0.0)
    discard;

  float visibility = texture2D(texture_1, gl_TexCoord[1].xy).r;
  // Don't render fogged terrain
  if(visibility == 0.0)
    discard;

  // Normalize light and view vectors and calculate halfangle vector
  //vec3 lightv = normalize(tLightDir);
  //vec3 viewv = normalize(tCameraDir);
  //vec3 halfv = normalize(viewv + lightv);

  // Get height from normalmap (for parallax mapping)
  vec2 texcoord = gl_TexCoord[0].xy;
  float height = texture2D(texture_2, texcoord).a * bumpScale - bumpBias;
  texcoord += (tCameraDir.xy * height);

  // Get the surface normal from normalmap
  vec3 normal = texture2D(texture_2, texcoord).rgb * 2.0 - 1.0;

  // Diffuse light received by surface (N.L)
  vec3 diffuse = dot(normal, tLightDir) * gl_LightSource[0].diffuse.rgb;

  // Amount of specular light
  //float specular = clamp(pow(dot(halfv, normal), 32), 0.0, 1.0);
  // Specular color
  //vec4 speccolor = gl_LightSource[0].specular * (specular * 0.2);

  // Color from the diffuse texture
  vec3 basetexcolor = texture2D(texture_0, texcoord).rgb;

  vec3 litcolor = basetexcolor * (diffuse + gl_LightSource[0].ambient.rgb);

  float fog = 1.0;
  if(fogEnabled)
    fog = clamp((gl_Fog.end - gl_FogFragCoord) * gl_Fog.scale, 0.0, 1.0);

  gl_FragColor = vec4(mix(gl_Fog.color.rgb, litcolor * visibility, fog), alpha) /*+ speccolor*/;
}

