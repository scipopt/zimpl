tar -cvzf zimpl-2.00.tgz \
--exclude="*CVS*" \
--exclude="*cvs*" \
--exclude="*~" \
--exclude="*.o" \
--exclude="*.d" \
--exclude="*TODO" \
--exclude="lint.out" \
--exclude=".\#*" \
zimpl/LICENSE zimpl/README zimpl/COPYING zimpl/CHANGELOG \
zimpl/Makefile zimpl/make zimpl/src zimpl/doc/*.pdf zimpl/check

