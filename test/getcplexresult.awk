/zimpl/ { name[$4] = $5; }
NF == 2  { var[name[$1]] = $2; }
/Current MIP best bound/ { lower = $6; }
/integer feasible:  Objective/ { upper = ($4 == "Objective") ? $6 : $7; }
/Solution time/ { time = $4; iters = $8; nodes = $11; }
END      {
   printf("%f %f %f %d %d\n", lower, upper, time, iters, nodes);
   
   for(v in var) {
      split(v, a, /[_@]/);
      if ((var[v] == 1.0) && ((a[1] == "xuv") || (a[1] == "xvh")))
	 if (substr(a[2], 3) != substr(a[3], 3))
	    print substr(a[2], 3), substr(a[3], 3);
   }
}
      
