--
-- Setup for segspace.sql tests
-- 

-- start_ignore
\! gpconfig -c gp_workfile_limit_per_segment -v 5242880
\! gpstop -rai
-- end_ignore
