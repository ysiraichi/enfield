OPENQASM 2.0;
include "qelib1.inc";

qreg a[2];
qreg b[2];

h b[1];
cx a[1], b[0];
cx b[0], b[1];

tdg b[1];
cx a[0], b[1];
t b[1];

cx b[0], b[1];
t b[0];

tdg b[1];
cx a[0], b[1];
t b[1];

cx a[0], b[0];
t a[0];
tdg b[0];
h b[1];
