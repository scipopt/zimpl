# check.zpl

param pi    := 4 * atan(1);
param sin1  := sin(1);
param cos1  := cos(1);
param tan1  := tan(1);
param asin1 := asin(sin1);
param acos1 := acos(cos1);
param atan1 := atan(tan1);

var d >= 0;
var x_pi;
var x_sin;
var x_cos;
var x_tan;
var x_asin;
var x_acos;
var x_atan;

minimize slack: d;

subto c1a: x_pi   + d >= pi;
subto c1b: x_pi   - d <= pi;
subto c2a: x_sin  + d >= sin1;
subto c2b: x_sin  - d <= sin1;
subto c3a: x_cos  + d >= cos1;
subto c3b: x_cos  - d <= cos1;
subto c4a: x_tan  + d >= tan1;
subto c4b: x_tan  - d <= tan1;
subto c5a: x_asin + d >= asin1;
subto c5b: x_asin - d <= asin1;
subto c6a: x_acos + d >= acos1;
subto c6b: x_acos - d <= acos1;
subto c7a: x_atan + d >= atan1;
subto c7b: x_atan - d <= atan1;
