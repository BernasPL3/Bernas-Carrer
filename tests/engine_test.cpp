#include "career.hpp"
#include <cassert>
#include <iostream>
int main(int argc,char**){
 using namespace bc;
 bool pairs[8][8]={};
 for(int r=0;r<14;r++){bool seen[8]={};for(auto f:fixtures(r)){assert(f.home!=f.away&&!seen[f.home]&&!seen[f.away]);assert(!pairs[f.home][f.away]);pairs[f.home][f.away]=true;seen[f.home]=seen[f.away]=true;}}
 for(int h=0;h<8;h++)for(int a=0;a<8;a++)if(a!=h)assert(pairs[h][a]);
 State s=create("Parity Player",1,0,123456789u);
 for(int season=0;season<6;season++){
  for(int r=0;r<14;r++){
   int type=s.energy<40?3:r%3;assert(train(s,type));State after=s;assert(!train(s,3));assert(serialize(s)==serialize(after));
   assert(simulate(s));if(s.offer>=0)acceptOffer(s);
   int points=0,draws=0;for(auto t:s.table){assert(t.played==s.round);assert(t.w+t.d+t.l==s.round);points+=t.pts();draws+=t.d;}
   assert(points==s.round*12-draws/2);assert(s.energy>=0&&s.energy<=100);
   State restored;assert(deserialize(serialize(s),restored));assert(serialize(s)==serialize(restored));
  }
  assert(!simulate(s));assert(nextSeason(s));
 }
 State unchanged=s;assert(!deserialize("BERNASCAREER 1\nbroken\n1 9 99",s));assert(serialize(s)==serialize(unchanged));
 if(argc>1)std::cout<<serialize(s);else std::cout<<"PASS: native engine, fixtures, tables, training limits, six seasons, save round trips and invalid saves.\n";
}
