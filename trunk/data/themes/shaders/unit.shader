<vertex>

uniform bool fogEnabled;

varying vec3 normal;
varying vec3 tolight;
varying vec3 tocamera;
varying vec3 color;
varying float fogStrength;

vec3 eucleidian(vec4 h) { return vec3(h.xyz / h.w); }

void main()
{
  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

  color = gl_Color.rgb;

  // Transform everything into eye space. We can't use untranformed stuff
  //  because models use matrix transformations.
  normal = gl_NormalMatrix * gl_Normal;
  vec4 eyevertex = gl_ModelViewMatrix * gl_Vertex;
  // FIXME: this assumes we're using directional light
  tolight = normalize(vec3(gl_LightSource[0].position));
#ifdef USE_SPECULAR
  // Camera is at (0; 0; 0) in eye space, so this is the direction to it.
  vec3 vertex = eucleidian(eyevertex);
  tocamera = normalize(vec3(-vertex));
#endif


  gl_TexCoord[1].s = dot(eyevertex, gl_EyePlaneS[3]);
  gl_TexCoord[1].t = dot(eyevertex, gl_EyePlaneT[3]);
  gl_TexCoord[1].p = dot(eyevertex, gl_EyePlaneR[3]);
  gl_TexCoord[1].q = dot(eyevertex, gl_EyePlaneQ[3]);
  gl_TexCoord[1] = gl_TextureMatrix[3] * gl_TexCoord[1];

  gl_Position = ftransform();

  if(fogEnabled)
    fogStrength = clamp((gl_Position.z - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0);
}

<fragment>
//#define USE_MATERIALS
//#define MAKE_IT_FAST

uniform sampler2D texture_0;
uniform sampler2DShadow texture_3;

uniform bool fogEnabled;

varying vec3 normal;
varying vec3 tolight;
varying vec3 tocamera;
varying vec3 color;
varying float fogStrength;

float specularStrength(vec3 lightdir, vec3 cameradir, vec3 normal)
{
  // Calculate specular light strength
  vec3 halfv = normalize(lightdir + cameradir);
#ifdef USE_MATERIALS
  return clamp(pow(dot(halfv, normal), gl_FrontMaterial.shininess), 0.0, 1.0);
#else
  return clamp(pow(dot(halfv, normal), 64.0), 0.0, 1.0);
#endif
}

void main()
{
  // Normalize the vectors
  vec3 mynormal = normalize(normal);
  // We could normalize those as well on faster cards
  vec3 lightdir = tolight;

  // Diffuse light strength
  float NdotL = max(0.0, dot(mynormal, lightdir));

#ifdef USE_SPECULAR
  float specular = specularStrength(lightdir, tocamera, mynormal);
#endif
  // Get diffuse texture color at this point
  vec4 texcolor = texture2D(texture_0, gl_TexCoord[0].xy);

  // Get shadow strength at this point
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
  float shadow = shadow2DProj(texture_3, gl_TexCoord[1]).r;
#else
  // 3-sample PCF filtering
  const float ires = 1.0 / 2048.0;
  vec3 spot = gl_TexCoord[1].stp / gl_TexCoord[1].q;
  float shadow = shadow2D(texture_3, vec3(spot.s - ires, spot.t + ires, spot.p)).r;
  shadow += shadow2D(texture_3, vec3(spot.s + ires, spot.t + ires, spot.p)).r;
  shadow += shadow2D(texture_3, vec3(spot.s, spot.t - ires, spot.p)).r;
  shadow /= 3.0;
#endif
#endif


  // Calculate final color of the surface
#ifdef USE_MATERIALS
  vec3 litcolor = (gl_FrontMaterial.ambient.rgb * gl_LightSource[0].ambient.rgb +
      shadow * NdotL * gl_FrontMaterial.diffuse.rgb * gl_LightSource[0].diffuse.rgb) * texcolor.rgb * color;
#ifdef USE_SPECULAR
  litcolor += shadow * specular * gl_FrontMaterial.specular.rgb * gl_LightSource[0].specular.rgb;
#endif
#else
  vec3 litcolor = (gl_LightSource[0].ambient.rgb +
      shadow * NdotL * gl_LightSource[0].diffuse.rgb) * texcolor.rgb * color;
#ifdef USE_SPECULAR
  litcolor += shadow * specular * gl_LightSource[0].specular.rgb;
#endif
#endif

  if(fogEnabled)
    litcolor = mix(litcolor, gl_Fog.color.rgb, fogStrength);

  gl_FragColor = vec4(litcolor, texcolor.a);
}

