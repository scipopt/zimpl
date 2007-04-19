tar -cvzhf zimpl-2.05.tgz \
--exclude="*CVS*" \
--exclude="*cvs*" \
--exclude="*~" \
--exclude="*.o" \
--exclude="*.d" \
--exclude="*TODO" \
--exclude="lint.out" \
--exclude=".\#*" \
--exclude="zimpl/make/local/*" \
--exclude="*.output" \
zimpl/LICENSE zimpl/README zimpl/CHANGELOG \
zimpl/Makefile zimpl/make zimpl/src zimpl/doc/*.pdf \
zimpl/doc/zimpl.man zimpl/check \
zimpl/example/chvatal_diet.zpl zimpl/example/facility_location.zpl \
zimpl/example/queens[12345].zpl zimpl/example/steinerbaum.zpl \
zimpl/example/tsp.zpl zimpl/example/tsp.dat

