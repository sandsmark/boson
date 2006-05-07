<vertex>


uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform bool fogEnabled;

varying float alpha;
varying vec3 tLightDir;
varying vec3 tCameraDir;
varying float fogStrength;

void main()
{
  // Stuff that will be sent to fragment shader
  alpha = gl_Color.w;

  // Texture coordinate for accessing diffuse and normalmap textures
  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_Vertex;

  // Texture coordinate for accessing fog texture
  gl_TexCoord[1] = gl_TextureMatrix[1] * gl_Vertex;
  gl_TexCoord[3] = gl_TextureMatrix[3] * gl_Vertex;

  // Calculate tangent and binormal
  vec3 tangent = normalize(vec3(gl_Normal.z, 0.0, -gl_Normal.x));
  vec3 binormal = normalize(vec3(0.0, gl_Normal.z, -gl_Normal.y));
  // Calculate tangent-space light- and view-vectors
  vec3 l = lightPos - gl_Vertex.xyz;
  vec3 v = cameraPos - gl_Vertex.xyz;
  tLightDir = normalize(vec3(dot(l, tangent),  dot(l, binormal),  dot(l, gl_Normal)));
  tCameraDir = normalize(vec3(dot(v, tangent),  dot(v, binormal),  dot(v, gl_Normal)));

  gl_Position = ftransform();

  if(fogEnabled)
    fogStrength = clamp((gl_Position.z - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0);
}


<fragment>

// Diffuse map
uniform sampler2D texture_0;
// Fog map
uniform sampler2D texture_1;
// Bumpmap
uniform sampler2D texture_2;
// Shadowmap
uniform sampler2DShadow texture_3;

uniform float bumpScale;
uniform float bumpBias;
uniform bool fogEnabled;

varying float alpha;
varying vec3 tLightDir;
varying vec3 tCameraDir;
varying float fogStrength;

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
#ifdef USE_PARALLAX_MAPPING
  float height = texture2D(texture_2, texcoord).a * bumpScale - bumpBias;
  texcoord += (tCameraDir.xy * height);
#endif

  // Get the surface normal from normalmap
  vec3 normal = texture2D(texture_2, texcoord).rgb * 2.0 - 1.0;

  // Diffuse light received by surface (N.L)
  vec3 diffuse = max(dot(normal, tLightDir), 0.0) * gl_LightSource[0].diffuse.rgb;

  // Amount of specular light
  //float specular = clamp(pow(dot(halfv, normal), 32), 0.0, 1.0);
  // Specular color
  //vec4 speccolor = gl_LightSource[0].specular * (specular * 0.2);

  // Color from the diffuse texture
  vec3 basetexcolor = texture2D(texture_0, texcoord).rgb;

#ifdef USE_HQ_PCF_SHADOWS
  float shadow = 0.0;
  const float ires = 1.0 / 2048.0;
  vec3 spot = gl_TexCoord[3].stp / gl_TexCoord[3].q;
  for(float x = -2.0; x <= 2.0; x++)
  {
    for(float y = -2.0; y <= 2.0; y++)
    {
      shadow += shadow2D(texture_3, vec3(spot.s + x*ires, spot.t + y*ires, spot.p)).r;
    }
  }
  shadow /= 25.0;
#else
#ifndef USE_PCF_SHADOWS
  float shadow = shadow2DProj(texture_3, gl_TexCoord[3]).r;
#else
  // 5-sample PCF filtering
  vec3 spot = gl_TexCoord[3].stp / gl_TexCoord[3].q;
  float ires = 1.0 / 2048.0;
  float shadow = 0.0;
  shadow += shadow2D(texture_3, vec3(spot.s       , spot.t - ires, spot.p)).r;
  shadow += shadow2D(texture_3, vec3(spot.s - ires, spot.t       , spot.p)).r;
  shadow += shadow2D(texture_3, vec3(spot.s       , spot.t       , spot.p)).r * 2.0;
  shadow += shadow2D(texture_3, vec3(spot.s + ires, spot.t       , spot.p)).r;
  shadow += shadow2D(texture_3, vec3(spot.s       , spot.t + ires, spot.p)).r;
  shadow = (shadow / 6.0);
#endif
#endif

  vec3 litcolor = basetexcolor * (diffuse * shadow + gl_LightSource[0].ambient.rgb);

  if(fogEnabled)
    litcolor = mix(litcolor, gl_Fog.color.rgb, fogStrength);

  gl_FragColor = vec4(litcolor * visibility, alpha) /*+ speccolor*/;
}

