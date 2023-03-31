//Maya ASCII 2010 scene
requires maya "2010";
currentUnit -l meter -a degree -t film;
createNode script -n "upAxisScriptNode";
	setAttr ".b" -type "string" "string $currentAxis = `upAxis -q -ax`; if ($currentAxis != \"y\") { upAxis -ax \"y\"; viewSet -home persp; }";
	setAttr ".st" 2;
createNode transform -n "polySurface13";
	addAttr -ln "colladaId" -dt "string";
	setAttr .colladaId -type "string" "polySurface13";
createNode phong -n "arch01_middlePiece_blinn4";
	setAttr ".dc" 1.000000;
	setAttr ".cp" 1.245731;
	addAttr -ln "colladaEffectId" -dt "string";
	setAttr .colladaEffectId -type "string" "blinn4-fx";
	addAttr -ln "colladaMaterialId" -dt "string";
	setAttr .colladaMaterialId -type "string" "blinn4";
createNode shadingEngine -n "arch01_middlePiece_blinn4SG";
createNode materialInfo -n "materialInfo_1";
createNode place2dTexture -n "place2dTexture_1";
createNode place2dTexture -n "place2dTexture_2";
createNode place2dTexture -n "place2dTexture_3";
createNode file -n "arch01_middlePiece_file13";
	setAttr ".ftn" -type "string" "./arch01_roughness_AO_emissive.dds";
	addAttr -ln "colladaId" -dt "string";
	setAttr .colladaId -type "string" "file13";
createNode file -n "arch01_middlePiece_file12";
	setAttr ".ftn" -type "string" "./arch01_albedo.dds";
	addAttr -ln "colladaId" -dt "string";
	setAttr .colladaId -type "string" "file12";
createNode file -n "arch01_middlePiece_file15";
	setAttr ".ftn" -type "string" "./arch01_substance.dds";
	addAttr -ln "colladaId" -dt "string";
	setAttr .colladaId -type "string" "file15";
createNode mesh -n "polySurfaceShape13" -p "|polySurface13";
	addAttr -ln "colladaId" -dt "string";
	setAttr .colladaId -type "string" "polySurfaceShape2";
	setAttr ".vt[0:23]" 0.000000 0.000000 0.000000 0.000000 -0.802608 0.000000 0.000000 -0.802608 -1.995028 0.000000 0.000000 -1.995028 0.000000 0.000000 -1.798507 0.000000 -0.905527 -1.798507 0.000000 -0.905527 -0.181471 0.000000 0.000000 -0.181471 -0.502598 0.000000 -1.798507 -0.502598 0.000000 -1.995028 -0.502598 -0.802608 -1.995028 -0.502598 -0.905527 -1.798507 -0.502598 -0.905527 -0.181471 -0.502598 -0.802608 0.000000 -0.502598 0.000000 0.000000 -0.502598 0.000000 -0.181471 -1.000000 -0.802608 0.000000 -1.000000 -0.905527 -0.181471 -1.000000 0.000000 -0.181471 -1.000000 0.000000 0.000000 -1.000000 -0.905527 -1.798507 -1.000000 0.000000 -1.798507 -1.000000 -0.802608 -1.995028 -1.000000 0.000000 -1.995028;
	setAttr ".n[0:131]" 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 -0.885871 -0.463931 0.000000 -0.885871 -0.463931 0.000000 -0.885871 -0.463931 0.000000 -0.885870 -0.463933 0.000000 -0.885870 -0.463933 0.000000 -0.885870 -0.463933 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -0.869848 0.493321 0.000000 -0.869848 0.493321 0.000000 -0.869848 0.493321 0.000000 -0.869846 0.493323 0.000000 -0.869846 0.493323 0.000000 -0.869846 0.493323 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 -0.885870 -0.463933 0.000000 -0.885870 -0.463933 0.000000 -0.885870 -0.463933 0.000000 -0.885871 -0.463931 0.000000 -0.885871 -0.463931 0.000000 -0.885871 -0.463931 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 -0.869847 0.493321 0.000000 -0.869847 0.493321 0.000000 -0.869847 0.493321 0.000000 -0.869846 0.493323 0.000000 -0.869846 0.493323 0.000000 -0.869846 0.493323 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.000000;
	setAttr ".usz" 1.000000;
	setAttr ".uvst[0].uvsn" -type "string" "polySurfaceShape2-polySurfaceShape2-polySurfaceShape2-polySurface11Shape-map1";
	setAttr ".cuvs" -type "string" "polySurfaceShape2-polySurfaceShape2-polySurfaceShape2-polySurface11Shape-map1";
	setAttr ".uvst[0].uvsp[0:71]" 0.902640 0.173786 0.931082 0.220936 0.683713 0.219222 0.684049 0.172223 0.928338 0.586513 0.681098 0.584799 0.899788 0.634820 0.680751 0.633253 0.671512 0.589171 0.671192 0.633891 0.570990 0.633204 0.571310 0.588457 0.687356 0.052589 0.904324 0.054141 0.903597 0.155750 0.686630 0.154198 0.997220 0.689243 0.968081 0.713560 0.910776 0.644893 0.939916 0.620575 0.966163 0.926525 0.600017 0.924385 0.600787 0.792674 0.966933 0.794813 0.936611 0.176375 0.912087 0.156916 0.959521 0.097136 0.984044 0.116594 0.895980 0.754971 0.685674 0.753466 0.686446 0.645516 0.896752 0.647021 0.674506 0.170712 0.674211 0.211876 0.574009 0.211163 0.574303 0.170022 0.902640 0.173786 0.684049 0.172223 0.683713 0.219222 0.931082 0.220936 0.681098 0.584799 0.928338 0.586513 0.899788 0.634820 0.680751 0.633253 0.671512 0.589171 0.571310 0.588457 0.570990 0.633204 0.671192 0.633891 0.687356 0.052589 0.686630 0.154198 0.903597 0.155750 0.904324 0.054141 0.997220 0.689243 0.939916 0.620575 0.910776 0.644893 0.968081 0.713560 0.966163 0.926525 0.966933 0.794813 0.600787 0.792674 0.600017 0.924385 0.936611 0.176375 0.984044 0.116594 0.959521 0.097136 0.912087 0.156916 0.895980 0.754971 0.896752 0.647021 0.686446 0.645516 0.685674 0.753466 0.674506 0.170712 0.574303 0.170022 0.574009 0.211163 0.674211 0.211876;
	setAttr ".ed[0:65]" 6 7 0 1 7 0 1 6 0 0 7 0 0 1 0 4 7 0 5 7 0 4 5 0 5 6 0 2 3 0 3 4 0 2 4 0 2 5 0 3 8 0 4 8 0 3 9 0 8 9 0 9 10 0 3 10 0 2 10 0 2 11 0 10 11 0 5 11 0 11 12 0 5 12 0 6 12 0 1 13 0 6 13 0 12 13 0 13 14 0 1 14 0 0 14 0 0 15 0 7 15 0 14 15 0 4 15 0 8 15 0 17 18 0 16 17 0 16 18 0 16 19 0 18 19 0 18 20 0 18 21 0 20 21 0 17 20 0 21 22 0 20 22 0 22 23 0 21 23 0 8 23 0 9 23 0 8 21 0 9 22 0 10 22 0 11 22 0 11 20 0 12 20 0 12 17 0 12 16 0 13 16 0 13 19 0 14 19 0 14 18 0 15 18 0 15 21 0;
	setAttr ".fc[0:43]" -type "polyFaces"  
		f 3 0 -2 2
		mu 0 3 1 2 0 
		f 3 1 -4 4
		mu 0 3 0 2 3 
		f 3 5 -7 -8
		mu 0 3 5 2 4 
		f 3 6 -1 -9
		mu 0 3 4 2 1 
		f 3 9 10 -12
		mu 0 3 6 7 5 
		f 3 11 7 -13
		mu 0 3 6 5 4 
		f 3 13 -15 -11
		mu 0 3 9 11 8 
		f 3 -14 15 -17
		mu 0 3 11 9 10 
		f 3 -18 -16 18
		mu 0 3 14 15 12 
		f 3 -10 19 -19
		mu 0 3 12 13 14 
		f 3 20 -22 -20
		mu 0 3 18 16 17 
		f 3 -21 12 22
		mu 0 3 16 18 19 
		f 3 -24 -23 24
		mu 0 3 22 23 20 
		f 3 8 25 -25
		mu 0 3 20 21 22 
		f 3 26 -28 -3
		mu 0 3 25 26 24 
		f 3 -26 27 -29
		mu 0 3 27 24 26 
		f 3 -30 -27 30
		mu 0 3 30 31 28 
		f 3 -5 31 -31
		mu 0 3 28 29 30 
		f 3 -33 3 33
		mu 0 3 34 32 33 
		f 3 32 -35 -32
		mu 0 3 32 34 35 
		f 3 35 -34 -6
		mu 0 3 8 34 33 
		f 3 -36 14 36
		mu 0 3 34 8 11 
		f 3 -38 -39 39
		mu 0 3 38 39 36 
		f 3 -40 40 -42
		mu 0 3 38 36 37 
		f 3 -43 43 -45
		mu 0 3 41 38 40 
		f 3 42 -46 37
		mu 0 3 38 41 39 
		f 3 44 46 -48
		mu 0 3 41 40 42 
		f 3 -49 -47 49
		mu 0 3 43 42 40 
		f 3 -51 16 51
		mu 0 3 47 45 46 
		f 3 50 -50 -53
		mu 0 3 45 47 44 
		f 3 53 48 -52
		mu 0 3 49 51 48 
		f 3 -54 17 54
		mu 0 3 51 49 50 
		f 3 -55 21 55
		mu 0 3 54 55 52 
		f 3 -56 56 47
		mu 0 3 54 52 53 
		f 3 23 57 -57
		mu 0 3 57 58 56 
		f 3 -58 58 45
		mu 0 3 56 58 59 
		f 3 59 38 -59
		mu 0 3 61 63 60 
		f 3 -60 28 60
		mu 0 3 63 61 62 
		f 3 61 -41 -61
		mu 0 3 65 67 64 
		f 3 -62 29 62
		mu 0 3 67 65 66 
		f 3 -64 34 64
		mu 0 3 71 69 70 
		f 3 63 41 -63
		mu 0 3 69 71 68 
		f 3 -66 -37 52
		mu 0 3 44 70 45 
		f 3 65 -44 -65
		mu 0 3 70 44 71;
connectAttr "arch01_middlePiece_blinn4SG.msg" "materialInfo_1.sg";
connectAttr "arch01_middlePiece_blinn4SG.pa" ":renderPartition.st" -na;
connectAttr "arch01_middlePiece_blinn4.oc" "arch01_middlePiece_blinn4SG.ss";
connectAttr "arch01_middlePiece_blinn4.msg" "materialInfo_1.m";
connectAttr "|polySurface13|polySurfaceShape13.iog" "arch01_middlePiece_blinn4SG.dsm" -na;
connectAttr "arch01_middlePiece_blinn4.msg" ":defaultShaderList1.s" -na;
connectAttr "defaultLightSet.msg" "lightLinker1.lnk[0].llnk";
connectAttr "arch01_middlePiece_blinn4SG.msg" "lightLinker1.lnk[0].olnk";
connectAttr "defaultLightSet.msg" "lightLinker1.slnk[0].sllk";
connectAttr "arch01_middlePiece_blinn4SG.msg" "lightLinker1.slnk[0].solk";
connectAttr "defaultLightSet.msg" "lightLinker1.lnk[1].llnk";
connectAttr ":initialShadingGroup.msg" "lightLinker1.lnk[1].olnk";
connectAttr "defaultLightSet.msg" "lightLinker1.slnk[1].sllk";
connectAttr ":initialShadingGroup.msg" "lightLinker1.slnk[1].solk";
connectAttr "lightLinker1.msg" ":lightList1.ln[0]";
connectAttr "place2dTexture_1.o" "arch01_middlePiece_file13.uv";
connectAttr "place2dTexture_1.ofs" "arch01_middlePiece_file13.fs";
connectAttr "place2dTexture_1.vt1" "arch01_middlePiece_file13.vt1";
connectAttr "place2dTexture_1.vt2" "arch01_middlePiece_file13.vt2";
connectAttr "place2dTexture_1.vt3" "arch01_middlePiece_file13.vt3";
connectAttr "place2dTexture_1.vc1" "arch01_middlePiece_file13.vc1";
connectAttr "place2dTexture_1.of" "arch01_middlePiece_file13.of";
connectAttr "place2dTexture_1.s" "arch01_middlePiece_file13.s";
connectAttr "place2dTexture_1.c" "arch01_middlePiece_file13.c";
connectAttr "place2dTexture_1.tf" "arch01_middlePiece_file13.tf";
connectAttr "place2dTexture_1.mu" "arch01_middlePiece_file13.mu";
connectAttr "place2dTexture_1.mv" "arch01_middlePiece_file13.mv";
connectAttr "place2dTexture_1.wu" "arch01_middlePiece_file13.wu";
connectAttr "place2dTexture_1.wv" "arch01_middlePiece_file13.wv";
connectAttr "place2dTexture_1.n" "arch01_middlePiece_file13.n";
connectAttr "place2dTexture_1.r" "arch01_middlePiece_file13.ro";
connectAttr "place2dTexture_1.re" "arch01_middlePiece_file13.re";
connectAttr "place2dTexture_2.o" "arch01_middlePiece_file12.uv";
connectAttr "place2dTexture_2.ofs" "arch01_middlePiece_file12.fs";
connectAttr "place2dTexture_2.vt1" "arch01_middlePiece_file12.vt1";
connectAttr "place2dTexture_2.vt2" "arch01_middlePiece_file12.vt2";
connectAttr "place2dTexture_2.vt3" "arch01_middlePiece_file12.vt3";
connectAttr "place2dTexture_2.vc1" "arch01_middlePiece_file12.vc1";
connectAttr "place2dTexture_2.of" "arch01_middlePiece_file12.of";
connectAttr "place2dTexture_2.s" "arch01_middlePiece_file12.s";
connectAttr "place2dTexture_2.c" "arch01_middlePiece_file12.c";
connectAttr "place2dTexture_2.tf" "arch01_middlePiece_file12.tf";
connectAttr "place2dTexture_2.mu" "arch01_middlePiece_file12.mu";
connectAttr "place2dTexture_2.mv" "arch01_middlePiece_file12.mv";
connectAttr "place2dTexture_2.wu" "arch01_middlePiece_file12.wu";
connectAttr "place2dTexture_2.wv" "arch01_middlePiece_file12.wv";
connectAttr "place2dTexture_2.n" "arch01_middlePiece_file12.n";
connectAttr "place2dTexture_2.r" "arch01_middlePiece_file12.ro";
connectAttr "place2dTexture_2.re" "arch01_middlePiece_file12.re";
connectAttr "place2dTexture_3.o" "arch01_middlePiece_file15.uv";
connectAttr "place2dTexture_3.ofs" "arch01_middlePiece_file15.fs";
connectAttr "place2dTexture_3.vt1" "arch01_middlePiece_file15.vt1";
connectAttr "place2dTexture_3.vt2" "arch01_middlePiece_file15.vt2";
connectAttr "place2dTexture_3.vt3" "arch01_middlePiece_file15.vt3";
connectAttr "place2dTexture_3.vc1" "arch01_middlePiece_file15.vc1";
connectAttr "place2dTexture_3.of" "arch01_middlePiece_file15.of";
connectAttr "place2dTexture_3.s" "arch01_middlePiece_file15.s";
connectAttr "place2dTexture_3.c" "arch01_middlePiece_file15.c";
connectAttr "place2dTexture_3.tf" "arch01_middlePiece_file15.tf";
connectAttr "place2dTexture_3.mu" "arch01_middlePiece_file15.mu";
connectAttr "place2dTexture_3.mv" "arch01_middlePiece_file15.mv";
connectAttr "place2dTexture_3.wu" "arch01_middlePiece_file15.wu";
connectAttr "place2dTexture_3.wv" "arch01_middlePiece_file15.wv";
connectAttr "place2dTexture_3.n" "arch01_middlePiece_file15.n";
connectAttr "place2dTexture_3.r" "arch01_middlePiece_file15.ro";
connectAttr "place2dTexture_3.re" "arch01_middlePiece_file15.re";
connectAttr "arch01_middlePiece_file13.msg" ":defaultTextureList1.tx" -na;
connectAttr "arch01_middlePiece_file13.oc" "arch01_middlePiece_blinn4.ambc";
connectAttr "arch01_middlePiece_file13.msg" "materialInfo_1.t" -na;
connectAttr "arch01_middlePiece_file12.msg" ":defaultTextureList1.tx" -na;
connectAttr "arch01_middlePiece_file12.oc" "arch01_middlePiece_blinn4.c";
connectAttr "arch01_middlePiece_file12.msg" "materialInfo_1.t" -na;
connectAttr "arch01_middlePiece_file15.msg" ":defaultTextureList1.tx" -na;
connectAttr "arch01_middlePiece_file15.oc" "arch01_middlePiece_blinn4.sc";
connectAttr "arch01_middlePiece_file15.msg" "materialInfo_1.t" -na;
