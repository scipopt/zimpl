#set HV   := { <"H1"> };
#set VV   := { <"V1">, <"V2"> };
#set UV   := { <"U1">, <"U2"> };
#set AHC  := HV * { <"Core"> };
#set AVH  := VV * HV; 
#set AUV  := UV * VV;

set K := { <"VV">, <"HV"> };

param dy[K] := <"VV"> 30000, <"HV"> 900000;
param dz[K] := <"VV"> 80000, <"HV"> 3000000;

var xuv[AUV] binary;
var xvh[AVH] binary;
var xhc[AHC] binary;   

var yvh[AVH] integer <= dy["VV"];
var yhc[AHC] integer <= dy["HV"];

var zvh[AVH] integer <= dz["VV"];
var zhc[AHC] integer <= dz["HV"];

#param cxuv[AUV]
#param cxvh[AVH]
#param cxhc[AHC]
#param cyvh[AVH]
#param czvh[AVH]
#param dyu[UV]
#param dzu[UV]

minimize cost: 
   sum <u,v> in AUV : cxuv[u,v] * xuv[u,v] 
 + sum <v,h> in AVH : cxvh[v,h] * xvh[v,h] 
 + sum <h,c> in AHC : cxhc[h,c] * xhc[h,c]
 + sum <v,h> in AVH : cyvh[v,h] * yvh[v,h]
 + sum <v,h> in AVH : czvh[v,h] * zvh[v,h];

# In die Senke muß mindestens einer reingehen
# 
subto c1: sum <h,c> in AHC : xhc[h,c] >= 1;

# Aus jeder uv muß genau einer rausgehen
#
subto c2: forall <u> in UV do sum <u,v> in AUV : xuv[u,v] == 1;

# Aus einer vm darf maximal einer rausgehen
#
subto c3: forall <v> in VV do sum <v,h> in AVH : xvh[v,h] <= 1 ;

# Es muß mindestens soviel in eine vm reingehen wie rausgeht
# 
subto c4: forall <v> in VV do 
   sum <u,v> in AUV : xuv[u,v] - sum <v,h> in AVH : xvh[v,h] >= 0;

# Es muß mindestens soviel in eine hv reingehen wie rausgeht.
# Bedingung (c3) brauchen wir nicht, weil es nur eine ausgehende Kante in
# für eine hv gibt.
#
subto c5: forall <h> in HV do
   sum <v,h> in AVH : xvh[v,h] - xhc[h,"Core"] >= 0;

# Wir binden die y Variablen an die x Variablen
#
subto c6: forall <v,h> in AVH do dy["VV"] * xvh[v,h] - yvh[v,h] >= 0;
subto c7: forall <h,c> in AHC do dy["HV"] * xhc[h,c] - yhc[h,c] >= 0;

# Wir binden die z Variablen an die x Variablen
#
subto c8: forall <v,h> in AVH do dz["VV"] * xvh[v,h] - zvh[v,h] >= 0;
subto c9: forall <h,c> in AHC do dz["HV"] * xhc[h,c] - zhc[h,c] >= 0;

# Flusserhaltung y: uv -> vm
#
subto c10: forall <v> in VV do 
     sum <u,v> in AUV : dyu[u] * xuv[u,v]
   - sum <v,h> in AVH : yvh[v,h] == 0;

# Flusserhaltung z: uv -> vm
#
subto c11: forall <v> in VV do 
     sum <u,v> in AUV : dzu[u] * xuv[u,v] 
   - sum <v,h> in AVH : zvh[v,h] == 0;

# Flusserhaltung y: vm ->hv
#
subto c12: 
   forall <h> in HV do sum <v,h> in AVH : yvh[v,h] - yhc[h,"Core"] == 0;

# Flusserhaltung z: vm ->hv
#
subto c13: 
   forall <h> in HV do sum <v,h> in AVH : zvh[v,h] - zhc[h,"Core"] == 0;

# Wenn ein VM an ist, muss die zugehörige UV auch an sie angeschlossen sein.
#
subto c14:
   forall <u,v> in UVEQ do sum <v,h> in AVH : xvh[v,h] - xuv[u,v] == 0;



