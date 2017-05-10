gate pre q { }   
gate post q { }  
qreg q[1];
creg c[1];
pre q[0];
barrier q;
h q[0];
barrier q;
post q[0];
measure q[0] -> c[0];
