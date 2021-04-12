// ##START_BOOSTRAP
let s_print = console.log;

// type information
let s_int = {
  'cast': v => v | 0
}
let s_float = {
  'cast': v => +v
}
let s_string = {
  'cast': v => new String(v)
}

// operators implementation
function so0 ()        { throw new Error("Invalid binary operation"); }
function so2 (   ...a) { return a.reduceRight((a,c) => Math.pow(c, a), 1); }
function so3 (   ...a) { return a.reduce((a,c) => a * c, 1); }
function so4 (l, ...a) { return a.reduce((a,c) => a / c, l); }
function so5 (l, ...a) { return a.reduce((a,c) => a % c, l); }
function so6 (l, ...a) { return a.reduce((a,c) => a + c, l); }
function so7 (l, ...a) { return a.reduce((a,c) => a - c, l); }

function so8 (l, r) { return l >= r; }
function so9 (l, r) { return l <= r; }
function so10(l, r) { return l >  r; }
function so11(l, r) { return l <  r; }
function so12(l, r) { return l == r; }
function so13(l, r) { return l != r; }

function so20(l,    t) { return t.cast(l); }

// ##END_BOOSTRAP
