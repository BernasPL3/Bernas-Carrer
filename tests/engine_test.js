'use strict';
const assert=require('node:assert/strict');
const cp=require('node:child_process');
const C=require('../web/engine.js');
let pairs=new Set();
for(let r=0;r<14;r++){let seen=new Set();for(const f of C.fixtures(r)){assert.notEqual(f.home,f.away);assert(!seen.has(f.home)&&!seen.has(f.away));seen.add(f.home);seen.add(f.away);const key=`${f.home},${f.away}`;assert(!pairs.has(key));pairs.add(key);}assert.equal(seen.size,8);}
assert.equal(pairs.size,56);
let s=C.create('Parity Player',1,0,123456789);
for(let season=0;season<6;season++){
 for(let r=0;r<14;r++){
  assert(C.train(s,s.energy<40?3:r%3));const before=C.serialize(s);assert(!C.train(s,3));assert.equal(before,C.serialize(s));
  assert(C.simulate(s));if(s.offer>=0)C.acceptOffer(s);
  let points=0,draws=0;for(const t of s.table){assert.equal(t.played,s.round);assert.equal(t.w+t.d+t.l,s.round);points+=C.pts(t);draws+=t.d;}
  assert.equal(points,s.round*12-draws/2);assert.equal(C.serialize(C.deserialize(C.serialize(s))),C.serialize(s));
 }
 assert(!C.simulate(s));assert(C.nextSeason(s));
}
assert.throws(()=>C.deserialize('BERNASCAREER 1\nbroken\n1 9 99'));
assert.throws(()=>C.deserialize(C.serialize(s)+'bad'));
const native=cp.execFileSync(process.argv[2]||'/tmp/bernas-career-tests',['golden'],{encoding:'utf8'});
assert.equal(C.serialize(s),native,'JavaScript and C++ must produce exactly the same save after six seasons');
console.log('PASS: web engine + exact C++/JS parity across 84 matches, six seasons, transfers and shared saves.');
