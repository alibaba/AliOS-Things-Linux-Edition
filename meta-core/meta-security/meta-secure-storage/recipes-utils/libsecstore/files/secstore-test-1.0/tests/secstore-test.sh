#!/bin/sh
ss_test0
su ssuser1 -c "ss_test1"
su ssuser1 -c "ss_test2"
su ssuser2 -c "ss_test3"
su ssuser3 -c "ss_test4"