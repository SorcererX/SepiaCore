//  Bayer Demosaicing 
//  by Morgan McGuire, Williams College
//  http://graphics.cs.williams.edu/papers/BayerJGT09/#shaders
//
// 	Ported to OpenCL by Ron Woods <rwoods@vaytek.com>

int Madd(int a, int b, int c)
{
  return a + b * c;
}

kernel void MHCdemosaic(__global read_only uchar *  source,
                         __global write_only uchar *  img)
{
#define fetch(x,y) source[Madd(x, y, width)]

  //In our sensor, lower-right is R
  int2 firstRed = (int2)(1, 1);

  //Gets information about work-item (which pixel to process)
  //Add offset for first red
  int4 center;
  center.xy = (int2)(get_global_id(0), get_global_id(1));
  center.zw =  center.xy + firstRed;

  //Gets information about work size (dimensions of mosaic)
  int width = get_global_size(0);
  int height = get_global_size(1);

  int4 xCoord = center.x + (int4)(-2, -1, 1, 2);
  int4 yCoord = center.y + (int4)(-2, -1, 1, 2);

  float C = fetch(center.x, center.y); // ( 0, 0)
  
  const float4 kC = (float4)(4.0f, 6.0f, 5.0f, 5.0f) / 8.0f;

  //Determine which of four types of pixels we are on.
  float2 alternate = fmod(floor(convert_float2(center.zw)), (float2)2.0f);

  float4 Dvec = (float4)(
	fetch(xCoord.s1, yCoord.s1),  // (-1,-1)
	fetch(xCoord.s1, yCoord.s2),  // (-1, 1)
	fetch(xCoord.s2, yCoord.s1),  // ( 1,-1)
	fetch(xCoord.s2, yCoord.s2)); // ( 1, 1)

  float4 PATTERN = kC * C;

  //Equivalent to:  
  //float D = Dvec.s0 + Dvec.s1 + Dvec.s2 + Dvec.s3;
  Dvec.xy += Dvec.zw;
  Dvec.x  += Dvec.y;
 
  float4 value = (float4)(
	fetch(center.x, yCoord.s0),  // ( 0,-2) A0
	fetch(center.x, yCoord.s1),  // ( 0,-1) B0
	fetch(xCoord.s0, center.y),  // (-2, 0) E0
	fetch(xCoord.s1, center.y)); // (-1, 0) F0

  float4 temp = (float4)(
	fetch(center.x, yCoord.s3),  // ( 0, 2) A1
	fetch(center.x, yCoord.s2),  // ( 0, 1) B1
	fetch(xCoord.s3, center.y),  // ( 2, 0) E1
	fetch(xCoord.s2, center.y)); // ( 1, 0) F1

  const float4 kA = (float4)(-1.0f, -1.5f, 0.5f, -1.0f) / 8.0f;
  const float4 kB = (float4)( 2.0f, 0.0f, 0.0f, 4.0f) / 8.0f;
  const float4 kD = (float4)( 0.0f, 2.0f, -1.0f, -1.0f) / 8.0f;

  // Conserve constant registers and take advantage of free swizzle on load
  #define kE kA.xywz
  #define kF kB.xywz

  value += temp;  // (A0 + A1), (B0 + B1), (E0 + E1), (F0 + F1)

  // There are five filter patterns (identity, cross, checker,
  // theta, phi). Precompute the terms from all of them and then
  // use swizzles to assign to color channels.
  //
  // Channel Matches
  // x cross (e.g., EE G)
  // y checker (e.g., EE B)
  // z theta (e.g., EO R)
  // w phi (e.g., EO B)

  #define A value.s0  // A0 + A1
  #define B value.s1  // B0 + B1
  #define D Dvec.x    // D0 + D1 + D2 + D3
  #define E value.s2  // E0 + E1
  #define F value.s3  // F0 + F1

  // Avoid zero elements. On a scalar processor this saves two MADDs and it has no
  // effect on a vector processor.
//PATTERN.yzw += (kD.yz * D).xyy;  <- invalid in OpenCL
  float4 kDtemp = (float4)(kD.yz * D, 0.0f, 0.0f);
  PATTERN.yzw += kDtemp.xyy;

//PATTERN += (kA.xyz * A).xyzx + (kE.xyw * E).xyxz;  <- invalid in OpenCL
  float4 kEtemp = (float4)(kE.xyw * E, 0.0f);
  PATTERN += kA * A + kEtemp.xyxz;

  PATTERN.xw += kB.xw * B;

  PATTERN.xz += kF.xz * F;

  // in RGB sequence; alpha = 1.0f included, because no float3 type in OpenCL 1.0
  float4 pixelColor = (alternate.y == 0.0f) ? 
	((alternate.x == 0.0f) ?
	(float4)(C, PATTERN.xy, 1.0f) : 
	(float4)(PATTERN.z, C, PATTERN.w, 1.0f)) :
	((alternate.x == 0.0f) ?
	(float4)(PATTERN.w, C, PATTERN.z, 1.0f) : 
	(float4)(PATTERN.yx, C, 1.0f));

  // output needed in BGR sequence; ignore alpha
  int ind = (center.x + center.y * width) * 3;
  img[  ind] = convert_uchar_sat(pixelColor.s2);	// B
  img[1+ind] = convert_uchar_sat(pixelColor.s1); 	// G
  img[2+ind] = convert_uchar_sat(pixelColor.s0);	// R
}