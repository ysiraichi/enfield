EFD_LIB(

// Swap qubits 'a' and 'b'.
gate intrinsic_swap__ a, b {
    cx a, b;
    cx b, a;
    cx a, b;
}

// Applies a cnot gate between qubits 'a' and 'b',
// such that '(a, c)' and '(c, b)' exists.
gate intrinsic_lcx__ a, c, b {
    cx c, b;
    cx a, c;
    cx c, b;
    cx a, c;
}

// Applies a reverse cnot from '(a, b)', using a
// cnot gate '(b, a)'.
gate intrinsic_rev_cx__ a, b {
    h a;
    h b;
    cx b, a;
    h b;
    h a;
}


)
