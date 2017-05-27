OPENQASM 2.0;
include "qelib1.inc";

qreg r[5];

cx r[3], r[1];
cx r[3], r[2];
cx r[2], r[1];
cx r[4], r[0];
cx r[4], r[1];
cx r[0], r[1];
cx r[0], r[3];
cx r[3], r[0];
cx r[0], r[3];
