# Page 5
var XB real;
var XC real;
maximize cost: 25 * XB + 30 * XC;
subto Time   : (1/200) * XB + (1/140) * XC <= 40;
subto B_limit: XB <= 6000; 
subto C_limit: XC <= 4000; 
