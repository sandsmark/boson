<vertex>

uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform bool fogEnabled;

varying vec3 normal;
varying vec3 vertex;
varying vec3 tolight;
varying vec3 tocamera;
varying vec3 color;

vec3 eucleidian(vec4 h) { return vec3(h.xyz / h.w); }

void main()
{
  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

  color = gl_Color.rgb;

  // Transform everything into eye space. We can't use untranformed stuff
  //  because models use matrix transformations.
  normal = gl_NormalMatrix * gl_Normal;
  vec4 eyevertex = gl_ModelViewMatrix * gl_Vertex;
  vertex = eucleidian(eyevertex);
  tolight = normalize(eucleidian(gl_ModelViewMatrix * vec4(lightPos, 1.0)) - vertex);
  // Camera is at (0; 0; 0) in eye space, so this is the direction to it.
  tocamera = normalize(vec3(-vertex));


  gl_TexCoord[1].s = dot(eyevertex, gl_EyePlaneS[3]);
  gl_TexCoord[1].t = dot(eyevertex, gl_EyePlaneT[3]);
  gl_TexCoord[1].p = dot(eyevertex, gl_EyePlaneR[3]);
  gl_TexCoord[1].q = dot(eyevertex, gl_EyePlaneQ[3]);
  gl_TexCoord[1] = gl_TextureMatrix[3] * gl_TexCoord[1];

  if(fogEnabled)
    gl_FogFragCoord = gl_Position.z;

  gl_Position = ftransform();
}

<fragment>
//#define USE_MATERIALS
//#define MAKE_IT_FAST

uniform sampler2D texture_0;
uniform sampler2DShadow texture_3;

uniform bool fogEnabled;

varying vec3 normal;
varying vec3 vertex;
varying vec3 tolight;
varying vec3 tocamera;
varying vec3 color;

void main()
{
  // Normalize the vectors
  vec3 mynormal = normalize(normal);
  // We could normalize those as well on faster cards
  vec3 lightdir = tolight;
  vec3 cameradir = tocamera;

  // Diffuse light strength
  float NdotL = max(0.0, dot(mynormal, lightdir));

#ifdef MAKE_IT_FAST
  const float specular = 0.0;
#else
  // Calculate specular light strength
  vec3 halfv = normalize(lightdir + cameradir);
#ifdef USE_MATERIALS
  float specular = clamp(pow(dot(halfv, mynormal), gl_FrontMaterial.shininess), 0.0, 1.0);
#else
  float specular = clamp(pow(dot(halfv, mynormal), 64.0), 0.0, 1.0);
#endif
#endif
  // Get diffuse texture color at this point
  vec4 texcolor = texture2D(texture_0, gl_TexCoord[0].xy);

  // Get shadow strength at this point
#ifdef MAKE_IT_FAST
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


  // Calculate final color of the surface
#ifdef USE_MATERIALS
  vec3 litcolor = (gl_FrontMaterial.ambient.rgb * gl_LightSource[0].ambient.rgb +
      shadow * NdotL * gl_FrontMaterial.diffuse.rgb * gl_LightSource[0].diffuse.rgb) * texcolor.rgb * color +
      shadow * specular * gl_FrontMaterial.specular.rgb * gl_LightSource[0].specular.rgb;
#else
  vec3 litcolor = (vec3(0.8, 0.8, 0.8) * gl_LightSource[0].ambient.rgb +
      shadow * NdotL * vec3(0.8, 0.8, 0.8) * gl_LightSource[0].diffuse.rgb) * texcolor.rgb * color +
      shadow * specular * vec3(0.8, 0.8, 0.8) * gl_LightSource[0].specular.rgb;
#endif

  float fog = 1.0;
  if(fogEnabled)
    fog = clamp((gl_Fog.end - gl_FogFragCoord) * gl_Fog.scale, 0.0, 1.0);

  gl_FragColor = vec4(mix(gl_Fog.color.rgb, litcolor, fog), texcolor.a);
}

