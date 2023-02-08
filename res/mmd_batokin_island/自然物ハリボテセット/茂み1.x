xof 0302txt 0064
template Header {
 <3D82AB43-62DA-11cf-AB39-0020AF71E433>
 WORD major;
 WORD minor;
 DWORD flags;
}

template Vector {
 <3D82AB5E-62DA-11cf-AB39-0020AF71E433>
 FLOAT x;
 FLOAT y;
 FLOAT z;
}

template Coords2d {
 <F6F23F44-7686-11cf-8F52-0040333594A3>
 FLOAT u;
 FLOAT v;
}

template Matrix4x4 {
 <F6F23F45-7686-11cf-8F52-0040333594A3>
 array FLOAT matrix[16];
}

template ColorRGBA {
 <35FF44E0-6C7C-11cf-8F52-0040333594A3>
 FLOAT red;
 FLOAT green;
 FLOAT blue;
 FLOAT alpha;
}

template ColorRGB {
 <D3E16E81-7835-11cf-8F52-0040333594A3>
 FLOAT red;
 FLOAT green;
 FLOAT blue;
}

template IndexedColor {
 <1630B820-7842-11cf-8F52-0040333594A3>
 DWORD index;
 ColorRGBA indexColor;
}

template Boolean {
 <4885AE61-78E8-11cf-8F52-0040333594A3>
 WORD truefalse;
}

template Boolean2d {
 <4885AE63-78E8-11cf-8F52-0040333594A3>
 Boolean u;
 Boolean v;
}

template MaterialWrap {
 <4885AE60-78E8-11cf-8F52-0040333594A3>
 Boolean u;
 Boolean v;
}

template TextureFilename {
 <A42790E1-7810-11cf-8F52-0040333594A3>
 STRING filename;
}

template Material {
 <3D82AB4D-62DA-11cf-AB39-0020AF71E433>
 ColorRGBA faceColor;
 FLOAT power;
 ColorRGB specularColor;
 ColorRGB emissiveColor;
 [...]
}

template MeshFace {
 <3D82AB5F-62DA-11cf-AB39-0020AF71E433>
 DWORD nFaceVertexIndices;
 array DWORD faceVertexIndices[nFaceVertexIndices];
}

template MeshFaceWraps {
 <4885AE62-78E8-11cf-8F52-0040333594A3>
 DWORD nFaceWrapValues;
 Boolean2d faceWrapValues;
}

template MeshTextureCoords {
 <F6F23F40-7686-11cf-8F52-0040333594A3>
 DWORD nTextureCoords;
 array Coords2d textureCoords[nTextureCoords];
}

template MeshMaterialList {
 <F6F23F42-7686-11cf-8F52-0040333594A3>
 DWORD nMaterials;
 DWORD nFaceIndexes;
 array DWORD faceIndexes[nFaceIndexes];
 [Material]
}

template MeshNormals {
 <F6F23F43-7686-11cf-8F52-0040333594A3>
 DWORD nNormals;
 array Vector normals[nNormals];
 DWORD nFaceNormals;
 array MeshFace faceNormals[nFaceNormals];
}

template MeshVertexColors {
 <1630B821-7842-11cf-8F52-0040333594A3>
 DWORD nVertexColors;
 array IndexedColor vertexColors[nVertexColors];
}

template Mesh {
 <3D82AB44-62DA-11cf-AB39-0020AF71E433>
 DWORD nVertices;
 array Vector vertices[nVertices];
 DWORD nFaces;
 array MeshFace faces[nFaces];
 [...]
}

Header{
1;
0;
1;
}

Mesh {
 16;
 -0.63307;2.21085;0.22681;,
 0.62180;2.21764;0.21311;,
 0.60393;1.50695;-0.24076;,
 -0.65094;1.50015;-0.22706;,
 -1.88484;1.45081;0.15283;,
 -0.64782;0.74691;-0.31475;,
 -1.88171;0.69757;0.06514;,
 0.60706;0.75371;-0.32845;,
 1.87981;1.47120;0.11171;,
 1.88293;0.71795;0.02401;,
 -0.64470;-0.00633;-0.40245;,
 -1.87860;-0.05567;-0.02257;,
 0.61018;0.00046;-0.41615;,
 1.88605;-0.03529;-0.06368;,
 -1.88796;2.20405;0.24052;,
 1.87669;2.22444;0.19941;;
 
 20;
 4;0,1,2,3;,
 4;4,3,5,6;,
 4;3,2,7,5;,
 4;2,8,9,7;,
 4;6,5,10,11;,
 4;5,7,12,10;,
 4;7,9,13,12;,
 4;4,3,0,14;,
 4;3,2,1,0;,
 4;2,8,15,1;,
 4;6,5,3,4;,
 4;5,7,2,3;,
 4;7,9,8,2;,
 4;11,10,5,6;,
 4;10,12,7,5;,
 4;12,13,9,7;,
 3;4,14,0;,
 3;4,0,3;,
 3;8,2,1;,
 3;8,1,15;;
 
 MeshMaterialList {
  1;
  20;
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0;;
  Material {
   0.000000;0.000000;0.000000;0.990000;;
   0.000000;
   0.000000;0.000000;0.000000;;
   1.000000;1.000000;1.000000;;
   TextureFilename {
    "tx\\tx_foliage1.tga";
   }
  }
 }
 MeshTextureCoords {
  16;
  0.333330;0.017490;,
  0.666670;0.017490;,
  0.666670;0.333330;,
  0.333330;0.333330;,
  0.012750;0.333330;,
  0.333330;0.666670;,
  0.012750;0.666670;,
  0.666670;0.666670;,
  0.995750;0.333330;,
  0.995750;0.666670;,
  0.333330;1.000000;,
  0.012750;1.000000;,
  0.666670;1.000000;,
  0.995750;1.000000;,
  0.012750;0.017490;,
  0.995750;0.017490;;
 }
}
