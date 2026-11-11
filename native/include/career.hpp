#pragma once
#include <algorithm>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace bc {
inline int clamp(int n,int lo,int hi){return std::max(lo,std::min(hi,n));}
struct Club { const char* name; const char* code; int strength; };
static const Club clubs[]={{"Middlesbrough","BOR",64},{"Ajax","AJA",73},{"Benfica","BEN",77},{"Dortmund","BVB",79},{"Porto","POR",75},{"Lyon","LYO",70},{"Fiorentina","FIO",72},{"Real Sociedad","RSO",76}};
[[maybe_unused]] static const char* positions[]={"Striker","Attacking midfielder","Left winger"};
[[maybe_unused]] static const char* attrs[]={"Pace","Shooting","Passing","Dribbling","Defending","Physical"};
struct Standing{int played=0,w=0,d=0,l=0,gf=0,ga=0;int pts()const{return w*3+d;}};
struct Fixture{int home,away;};
struct History{int year,team,goals,assists,rank;};
struct Match{int home=-1,away=-1,hg=0,ag=0,goals=0,assists=0,rating10=0;};
struct State {
 uint32_t rng=260627; std::string name="Bernardo";
 int position=0,team=0,season=2026,age=17,round=0,energy=94,morale=76,xp=0,reputation=10;
 int attributes[6]={64,59,58,62,36,61}; bool trained=false;
 int goals=0,assists=0,totalGoals=0,totalAssists=0,totalMatches=0,titles=0;
 int salary=1800,cash=0,offer=-1,pending=-1;
 Standing table[8]; Match last; std::vector<History> history;
 std::vector<std::string> news;
};
inline int rnd(State& s,int max){s.rng=s.rng*1664525u+1013904223u;return int((s.rng>>1)%uint32_t(max));}
inline int overall(const State&s){
 const int weights[3][6]={{15,35,10,20,0,20},{10,15,35,25,0,15},{30,20,15,25,0,10}};
 int total=0;for(int i=0;i<6;i++)total+=s.attributes[i]*weights[s.position][i];return total/100;
}
inline void news(State&s,const std::string& n){s.news.insert(s.news.begin(),n);if(s.news.size()>8)s.news.pop_back();}
inline std::vector<Fixture> fixtures(int round){
 int ids[8]={0,1,2,3,4,5,6,7};
 for(int r=0;r<round%7;r++){int last=ids[7];for(int i=7;i>1;i--)ids[i]=ids[i-1];ids[1]=last;}
 std::vector<Fixture> out;
 for(int i=0;i<4;i++){int h=ids[i],a=ids[7-i];if((round%7)%2)std::swap(h,a);if(round>=7)std::swap(h,a);out.push_back({h,a});}return out;
}
inline Fixture nextFixture(const State&s){for(auto f:fixtures(s.round))if(f.home==s.team||f.away==s.team)return f;return {0,1};}
inline std::vector<int> ranking(const State&s){std::vector<int> ids={0,1,2,3,4,5,6,7};std::sort(ids.begin(),ids.end(),[&](int a,int b){const auto&x=s.table[a];const auto&y=s.table[b];if(x.pts()!=y.pts())return x.pts()>y.pts();if(x.gf-x.ga!=y.gf-y.ga)return x.gf-x.ga>y.gf-y.ga;if(x.gf!=y.gf)return x.gf>y.gf;return a<b;});return ids;}
inline int rank(const State&s){auto r=ranking(s);return int(std::find(r.begin(),r.end(),s.team)-r.begin())+1;}
inline State create(const std::string& name="Bernardo",int pos=0,int team=0,uint32_t seed=260627){State s;s.name=name.empty()?"Bernardo":name.substr(0,24);for(char& c:s.name)if(c=='\n'||c=='\r')c=' ';s.position=clamp(pos,0,2);s.team=clamp(team,0,7);s.rng=seed;s.salary=1800+(clubs[s.team].strength-64)*100;news(s,"A new story begins. Earn your place!");return s;}
inline bool train(State&s,int type){
 if(s.round>=14||s.trained||type<0||type>3)return false;
 if(type!=3&&s.energy<25){news(s,"Low energy. Choose recovery this week.");return false;}
 if(type==3){s.energy=clamp(s.energy+32,0,100);s.morale=clamp(s.morale+3,0,100);news(s,"Recovery complete: +32 energy.");}
 else {const int pairs[3][2]={{1,3},{2,3},{0,5}};for(int j=0;j<2;j++){int a=pairs[type][j];s.attributes[a]=clamp(s.attributes[a]+1,1,99);}s.energy-=16;s.xp+=15;news(s,"Training complete: two attributes improved.");}
 s.trained=true;return true;
}
inline void result(Standing&h,Standing&a,int hg,int ag){h.played++;a.played++;h.gf+=hg;h.ga+=ag;a.gf+=ag;a.ga+=hg;if(hg>ag){h.w++;a.l++;}else if(hg<ag){a.w++;h.l++;}else{h.d++;a.d++;}}
inline int teamScore(State&s,int strength,int other){int score=0;int chance=clamp(31+(strength-other)/2,12,58);for(int k=0;k<4;k++)if(rnd(s,100)<chance)score++;return score;}
inline bool simulate(State&s){
 if(s.round>=14)return false;
 int ov=overall(s);Match own;
 for(auto f:fixtures(s.round)){
  int hs=clubs[f.home].strength+(f.home==s.team?(ov-60)/2+(s.energy-60)/12:0)+3;
  int as=clubs[f.away].strength+(f.away==s.team?(ov-60)/2+(s.energy-60)/12:0);
  int hg=teamScore(s,hs,as),ag=teamScore(s,as,hs);
  if(f.home==s.team||f.away==s.team){
   int ownGoals=f.home==s.team?hg:ag;
   int pg=0,pa=0;
   for(int g=0;g<ownGoals;g++){if(rnd(s,100)<clamp(20+s.attributes[1]/3+(s.position==0?10:0)+(s.energy-70)/5,10,75))pg++;else if(rnd(s,100)<clamp(15+s.attributes[2]/3+(s.position==1?14:0),15,70))pa++;}
   int win=(f.home==s.team?hg-ag:ag-hg);
   own={f.home,f.away,hg,ag,pg,pa,clamp(57+pg*12+pa*8+(win>0?7:win==0?2:-4)+rnd(s,9)+(s.energy-70)/10,40,100)};
   s.morale=clamp(s.morale+(win>0?5:win==0?1:-4),0,100);
  }
  result(s.table[f.home],s.table[f.away],hg,ag);
 }
 s.last=own;s.round++;s.totalMatches++;s.goals+=own.goals;s.assists+=own.assists;s.totalGoals+=own.goals;s.totalAssists+=own.assists;
 s.cash+=s.salary;s.xp+=25+own.goals*12+own.assists*8;s.reputation=clamp(s.reputation+own.goals*2+own.assists+(own.rating10>=70?2:1),0,100);
 while(s.xp>=100){s.xp-=100;int a=s.position==0?1:s.position==1?2:0;s.attributes[a]=clamp(s.attributes[a]+1,1,99);s.attributes[5]=clamp(s.attributes[5]+1,1,99);}
 s.energy=clamp(s.energy-22-rnd(s,11)+18,0,100);s.trained=false;
 std::ostringstream n;n<<clubs[own.home].code<<" "<<own.hg<<" x "<<own.ag<<" "<<clubs[own.away].code<<" | Rating "<<own.rating10/10<<"."<<own.rating10%10;news(s,n.str());
 if((s.round==4||s.round==8||s.round==12)&&s.pending<0){std::vector<int> candidates;for(int t=0;t<8;t++)if(t!=s.team&&clubs[t].strength<=ov+s.reputation/4+10)candidates.push_back(t);if(!candidates.empty()){s.offer=candidates[rnd(s,int(candidates.size()))];news(s,std::string("Offer received: ")+clubs[s.offer].name+". Check the transfer page.");}}
 if(s.round==14)news(s,"Season complete! Check the league and start the next one.");return true;
}
inline int proposedSalary(const State&s,int team){return std::max(s.salary+300,1800+(clubs[team].strength-64)*150+s.reputation*30);}
inline bool acceptOffer(State&s){if(s.offer<0||s.pending>=0)return false;s.pending=s.offer;s.offer=-1;news(s,std::string("Contract signed with ")+clubs[s.pending].name+" for next season.");return true;}
inline bool nextSeason(State&s){if(s.round!=14)return false;int r=rank(s);s.history.insert(s.history.begin(),{s.season,s.team,s.goals,s.assists,r});if(s.history.size()>8)s.history.pop_back();if(r==1)s.titles++;if(s.pending>=0){s.salary=proposedSalary(s,s.pending);s.team=s.pending;}s.pending=-1;s.offer=-1;s.season++;s.age++;s.round=0;s.goals=0;s.assists=0;s.energy=100;s.morale=80;s.trained=false;s.last=Match{};for(auto&t:s.table)t=Standing{};news(s,"New season! Your progress and history carry on.");return true;}
inline std::string serialize(const State&s){
 std::ostringstream o;o<<"BERNASCAREER 1\n"<<s.name<<"\n"<<s.rng<<" "<<s.position<<" "<<s.team<<" "<<s.season<<" "<<s.age<<" "<<s.round<<" "<<s.energy<<" "<<s.morale<<" "<<s.xp<<" "<<s.reputation<<" "<<int(s.trained)<<"\n";
 for(int a:s.attributes)o<<a<<" ";o<<"\n"<<s.goals<<" "<<s.assists<<" "<<s.totalGoals<<" "<<s.totalAssists<<" "<<s.totalMatches<<" "<<s.titles<<" "<<s.salary<<" "<<s.cash<<" "<<s.offer<<" "<<s.pending<<"\n";
 for(const auto&t:s.table)o<<t.played<<" "<<t.w<<" "<<t.d<<" "<<t.l<<" "<<t.gf<<" "<<t.ga<<"\n";
 auto m=s.last;o<<m.home<<" "<<m.away<<" "<<m.hg<<" "<<m.ag<<" "<<m.goals<<" "<<m.assists<<" "<<m.rating10<<"\n"<<s.history.size()<<"\n";
 for(const auto&h:s.history)o<<h.year<<" "<<h.team<<" "<<h.goals<<" "<<h.assists<<" "<<h.rank<<"\n";return o.str();
}
inline bool deserialize(const std::string& text,State&out){
 if(text.size()>16384)return false;std::istringstream i(text);std::string magic;std::getline(i,magic);if(magic!="BERNASCAREER 1")return false;State s;std::getline(i,s.name);int trained=0;
 i>>s.rng>>s.position>>s.team>>s.season>>s.age>>s.round>>s.energy>>s.morale>>s.xp>>s.reputation>>trained;s.trained=trained==1;
 for(int&a:s.attributes)i>>a;
 i>>s.goals>>s.assists>>s.totalGoals>>s.totalAssists>>s.totalMatches>>s.titles>>s.salary>>s.cash>>s.offer>>s.pending;
 if(!i||s.name.empty()||s.name.size()>24||s.position<0||s.position>2||s.team<0||s.team>7||s.round<0||s.round>14||s.season<2026||s.season>2126||s.age<17||s.age>117||s.energy<0||s.energy>100||s.morale<0||s.morale>100||s.xp<0||s.xp>199||s.reputation<0||s.reputation>100||trained<0||trained>1||s.offer < -1||s.offer>7||s.pending < -1||s.pending>7||s.salary<0||s.salary>1000000||s.cash<0||s.totalMatches<0||s.totalMatches>1414||s.totalGoals<0||s.totalAssists<0||s.goals<0||s.goals>56||s.assists<0||s.assists>56||s.titles<0||s.titles>101)return false;
 for(int a:s.attributes)if(a<1||a>99)return false;
 int gf=0,ga=0;for(auto&t:s.table){i>>t.played>>t.w>>t.d>>t.l>>t.gf>>t.ga;if(!i||t.played!=s.round||t.w<0||t.d<0||t.l<0||t.w+t.d+t.l!=t.played||t.gf<0||t.gf>56||t.ga<0||t.ga>56)return false;gf+=t.gf;ga+=t.ga;}if(gf!=ga)return false;
 auto&m=s.last;i>>m.home>>m.away>>m.hg>>m.ag>>m.goals>>m.assists>>m.rating10;if(!i||m.home < -1||m.home>7||m.away < -1||m.away>7||m.hg<0||m.hg>4||m.ag<0||m.ag>4||m.rating10<0||m.rating10>100||m.goals<0||m.goals>4||m.assists<0||m.assists>4)return false;
 int count=-1;i>>count;if(!i||count<0||count>8)return false;s.history.clear();for(int n=0;n<count;n++){History h{};i>>h.year>>h.team>>h.goals>>h.assists>>h.rank;if(!i||h.year<2026||h.year>=s.season||h.team<0||h.team>7||h.rank<1||h.rank>8||h.goals<0||h.goals>56||h.assists<0||h.assists>56)return false;s.history.push_back(h);}i>>std::ws;if(!i.eof())return false;news(s,"Career loaded. Enjoy the season!");out=s;return true;
}
}
