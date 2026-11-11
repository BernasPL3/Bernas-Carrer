#include "career.hpp"
#include "renderer.hpp"
#include <fat.h>
#include <ogc/lwp_watchdog.h>
#ifdef PLATFORM_WII
#include <wiiuse/wpad.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>

using namespace bc;
using namespace ui;
static volatile bool quitting=false;
static void resetCallback(u32,void*){quitting=true;}
#ifdef PLATFORM_WII
static void powerCallback(){quitting=true;}
#endif
enum Key{UP=1,DOWN=2,LEFT=4,RIGHT=8,A=16,B=32,SAVE=64,LOAD=128,QUIT=256};
static int mapPad(u32 b){return((b&PAD_BUTTON_UP)?UP:0)|((b&PAD_BUTTON_DOWN)?DOWN:0)|((b&PAD_BUTTON_LEFT)?LEFT:0)|((b&PAD_BUTTON_RIGHT)?RIGHT:0)|((b&PAD_BUTTON_A)?A:0)|((b&PAD_BUTTON_B)?B:0)|((b&PAD_BUTTON_START)?SAVE:0)|((b&PAD_BUTTON_X)?LOAD:0)|((b&PAD_TRIGGER_Z)?QUIT:0);}
static int input(){
 PAD_ScanPads();int pressed=mapPad(PAD_ButtonsDown(0)),held=mapPad(PAD_ButtonsHeld(0));
 int x=PAD_StickX(0),y=PAD_StickY(0);if(x>50)held|=RIGHT;if(x< -50)held|=LEFT;if(y>50)held|=UP;if(y< -50)held|=DOWN;
#ifdef PLATFORM_WII
 WPAD_ScanPads();u32 w=WPAD_ButtonsDown(0),h=WPAD_ButtonsHeld(0);
 auto mapWii=[](u32 b){return((b&WPAD_BUTTON_UP)?UP:0)|((b&WPAD_BUTTON_DOWN)?DOWN:0)|((b&WPAD_BUTTON_LEFT)?LEFT:0)|((b&WPAD_BUTTON_RIGHT)?RIGHT:0)|((b&WPAD_BUTTON_A)?A:0)|((b&WPAD_BUTTON_B)?B:0)|((b&WPAD_BUTTON_PLUS)?SAVE:0)|((b&WPAD_BUTTON_MINUS)?LOAD:0)|((b&WPAD_BUTTON_HOME)?QUIT:0);};
 pressed|=mapWii(w);held|=mapWii(h);
#endif
 static int previous=0,repeat=0;int dir=held&15;if(dir!=previous){pressed|=dir;repeat=0;}else if(dir&&++repeat>20&&repeat%5==0)pressed|=dir;previous=dir;return pressed;
}
static const char* paths[]={"sd:/bernas-career.sav","usb:/bernas-career.sav","carda:/bernas-career.sav","cardb:/bernas-career.sav","/bernas-career.sav"};
static std::string savePath;static bool fatReady=false;
static bool writeSave(const State&s){
 if(!fatReady||savePath.empty())return false;std::string tmp=savePath+".tmp",backup=savePath+".bak",data=serialize(s);
 FILE*f=std::fopen(tmp.c_str(),"wb");if(!f)return false;bool ok=std::fwrite(data.data(),1,data.size(),f)==data.size();if(std::fflush(f)!=0)ok=false;if(std::fclose(f)!=0)ok=false;
 if(!ok){std::remove(tmp.c_str());return false;}
 std::remove(backup.c_str());bool moved=std::rename(savePath.c_str(),backup.c_str())==0;
 if(std::rename(tmp.c_str(),savePath.c_str())!=0){if(moved)std::rename(backup.c_str(),savePath.c_str());std::remove(tmp.c_str());return false;}std::remove(backup.c_str());return true;
}
static bool readSave(State&s){if(!fatReady)return false;for(const char*p:paths){FILE*f=std::fopen(p,"rb");if(!f)continue;char bytes[16385];size_t n=std::fread(bytes,1,sizeof(bytes),f);bool ok=!std::ferror(f)&&n<sizeof(bytes);std::fclose(f);State candidate;if(ok&&deserialize(std::string(bytes,n),candidate)){s=candidate;savePath=p;return true;}}return false;}
static void findSavePath(){if(!fatReady)return;for(const char*p:paths){std::string tmp=std::string(p)+".tmp";FILE*f=std::fopen(tmp.c_str(),"wb");if(f){std::fclose(f);std::remove(tmp.c_str());savePath=p;return;}}}
static std::string num(int n){return std::to_string(n);}
static std::string euro(int n){return "EUR "+num(n);}
static void pill(Renderer&r,int x,int y,int w,const std::string&t,bool active){r.rect(x,y,w,26,active?lime:white);r.text(x+9,y+8,t,active?green:muted,1.15f);}
static void bar(Renderer&r,int x,int y,int w,int value,Color c=green){r.rect(x,y,w,4,line);r.rect(x,y,w*clamp(value,0,100)/100.0f,4,c);}
int main(){
 Renderer r;PAD_Init();SYS_SetResetCallback(resetCallback);
#ifdef PLATFORM_WII
 WPAD_Init();SYS_SetPowerCallback(powerCallback);WPAD_SetPowerButtonCallback([](s32){quitting=true;});
#endif
 fatReady=fatInitDefault();findSavePath();State s=create();State loaded;bool hasSave=readSave(loaded);
 int screen=0,selection=0,tab=0,trainSelection=0,setupRow=0,nameCursor=0;bool editing=false;std::string editName="Bernardo";
 std::string message=fatReady&&!savePath.empty()?"FAT READY. AUTOMATIC SAVING ENABLED.":"NO FAT DEVICE: THIS SESSION WILL NOT BE SAVED.";int messageTimer=240;
 auto banner=[&](const std::string&m){message=m;messageTimer=200;};
 auto autosave=[&](){if(writeSave(s)){hasSave=true;loaded=s;}else banner("COULD NOT SAVE. CHECK YOUR SD/FAT DEVICE.");};
 while(!quitting){int key=input();
  if(key&QUIT){quitting=true;continue;}
  if((key&SAVE)&&screen==3){banner(writeSave(s)?"CAREER SAVED.":"SAVE FAILED. CHECK YOUR SD/FAT DEVICE.");}
  if((key&LOAD)&&screen==3){State fresh;if(readSave(fresh)){s=fresh;banner("CAREER LOADED.");}else banner("NO VALID SAVE FOUND.");}
  if(screen==0){if(key&(UP|DOWN))selection=1-selection;if(key&A){if(selection==0){screen=1;setupRow=0;}else if(readSave(loaded)){s=loaded;screen=3;}else banner("NO SAVED CAREER FOUND.");}}
  else if(screen==1){
   if(editing){const std::string alphabet=" ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-";if(key&LEFT)nameCursor=(nameCursor+23)%24;if(key&RIGHT)nameCursor=(nameCursor+1)%24;if(key&(UP|DOWN)){editName.resize(24,' ');size_t index=alphabet.find(editName[nameCursor]);if(index==std::string::npos)index=0;index=(index+alphabet.size()+((key&UP)?1:-1))%alphabet.size();editName[nameCursor]=alphabet[index];}if(key&A){while(!editName.empty()&&editName.back()==' ')editName.pop_back();s.name=editName.empty()?"Bernardo":editName;editing=false;}if(key&B)editing=false;
   }else{if(key&UP)setupRow=(setupRow+3)%4;if(key&DOWN)setupRow=(setupRow+1)%4;if(setupRow==1&&(key&(LEFT|RIGHT|A)))s.position=(s.position+((key&LEFT)?2:1))%3;if(setupRow==2&&(key&(LEFT|RIGHT|A)))s.team=(s.team+((key&LEFT)?7:1))%8;if((key&A)&&setupRow==0){editing=true;editName=s.name;editName.resize(24,' ');nameCursor=0;}if((key&A)&&setupRow==3){s=create(s.name,s.position,s.team,uint32_t(gettime()));screen=3;tab=0;autosave();}if(key&B)screen=0;}
  }
  else if(screen==3){
   if(key&LEFT)tab=(tab+5)%6;if(key&RIGHT)tab=(tab+1)%6;
   if(key&B)tab=0;
   if(tab==0&&(key&A)){if(s.round==14){if(s.season<2126){nextSeason(s);autosave();banner("NEW SEASON. LET US GO!");}else banner("SEASON LIMIT REACHED. YOUR STORY IS SAVED.");}else{simulate(s);autosave();banner(clubs[s.last.home].code+std::string(" ")+num(s.last.hg)+" X "+num(s.last.ag)+" "+clubs[s.last.away].code+" | YOUR RATING "+num(s.last.rating10/10)+"."+num(s.last.rating10%10));}}
   if(tab==1){if(key&UP)trainSelection=(trainSelection+3)%4;if(key&DOWN)trainSelection=(trainSelection+1)%4;if(key&A){if(train(s,trainSelection)){autosave();banner("PREPARATION COMPLETE. SIMULATE THE NEXT MATCH.");}else banner(s.trained?"YOU ALREADY PREPARED FOR THIS MATCHDAY.":s.round>=14?"START THE NEW SEASON.":"LOW ENERGY. CHOOSE RECOVERY.");}}
   if(tab==3&&(key&A)){if(acceptOffer(s)){autosave();banner("CONTRACT SIGNED FOR NEXT SEASON.");}else banner(s.pending>=0?"TRANSFER ALREADY CONFIRMED.":"IMPROVE AND WATCH FOR MATCHDAYS 4, 8 AND 12.");}
   if(tab==5&&(key&A)){screen=0;selection=0;banner("THE SAVE IS REPLACED ONLY WHEN YOU START A NEW CAREER.");}
  }
  r.rect(0,0,640,480,bg);r.rect(0,0,640,60,green);r.text(32,23,"b. / BERNAS CAREER",lime,2.0f);
#ifdef PLATFORM_WII
  r.text(506,27,"WII EDITION",white,1.1f);
#else
  r.text(464,27,"GAMECUBE EDITION",white,1.1f);
#endif
  if(screen==0){
   r.text(48,107,"YOUR GAME. YOUR STORY.",muted,1.3f);r.text(48,143,"TIME TO MAKE",ink,3);r.text(48,176,"YOUR NAME.",ink,3);
   pill(r,48,241,390,"NEW CAREER",selection==0);pill(r,48,282,390,hasSave?"CONTINUE CAREER":"CONTINUE: NO SAVE FOUND",selection==1);
   r.text(48,342,"A: CONFIRM   D-PAD: SELECT",muted,1.2f);r.text(48,370,"8 CLUBS. 14 MATCHDAYS. YOUR OWN STORY.",green,1.2f);
  }else if(screen==1){
   r.text(32,92,"YOUR FIRST CONTRACT",muted,1.3f);r.text(32,117,"CREATE YOUR PLAYER",ink,2.5f);
   const std::string rows[]={"NAME: "+s.name,"POSITION: "+std::string(positions[s.position]),"CLUB: "+std::string(clubs[s.team].name),"START CAREER"};
   for(int i=0;i<4;i++){pill(r,32,173+i*43,576,i==3?"START CAREER":rows[i],setupRow==i);}
   if(editing){r.rect(32,161,576,227,white);r.text(48,179,"EDIT NAME",green,1.5f);r.text(48,211,"UP/DOWN: LETTER   LEFT/RIGHT: CURSOR",muted,1.1f);r.text(48,248,editName,ink,2);r.rect(48+nameCursor*12,266,10,3,green);r.text(48,294,"A: CONFIRM   B: CANCEL",green,1.2f);}
   r.text(32,383,"LEFT/RIGHT: CHANGE  A: CONFIRM  B: BACK",muted,1.2f);r.text(32,408,"STARTING REPLACES YOUR SAVED CAREER.",red,1.1f);
  }else{
   const char*tabs[]={"CAREER","TRAINING","LEAGUE","TRANSFERS","HISTORY","HELP"};for(int i=0;i<6;i++)pill(r,32+i*97,73,91,tabs[i],tab==i);
   if(tab==0){
    r.rect(32,115,372,150,green);r.text(47,130,"SEASON "+num(s.season)+"/"+num((s.season+1)%100)+" | MATCHDAY "+num(s.round)+"/14",lime,1.1f);
    r.text(47,157,s.round==14?"FULL TIME.":"TIME TO MAKE",white,2);r.text(47,180,s.round==14?num(rank(s))+" IN THE LEAGUE":"YOUR NAME.",white,2);
    std::string fixture;if(s.round<14){auto f=nextFixture(s);fixture=std::string(clubs[f.home].code)+" VS "+clubs[f.away].code;}else fixture="THE STORY CONTINUES";
    r.text(47,211,fixture,lime,1.4f);r.rect(47,235,240,20,lime);r.text(55,241,s.round==14?"A: NEW SEASON":"A: SIMULATE NEXT MATCH",green,1.1f);
    r.outline(305,147,77,96,{115,146,92});r.rect(305,194,77,1,{115,146,92});r.outline(327,147,34,18,{115,146,92});r.outline(327,225,34,18,{115,146,92});r.rect(354,171,5,5,lime);r.rect(326,208,5,5,lime);
    const std::string values[]={num(overall(s)),num(s.goals),num(s.assists),s.round?num(rank(s)):"-"};const char*labels[]={"OVERALL","GOALS","ASSIST.","PLACE"};
    for(int i=0;i<4;i++){int x=32+i*96;r.rect(x,278,84,64,white);r.text(x+10,289,labels[i],muted,1.05f);r.text(x+10,309,values[i],green,2.3f);}
    r.rect(32,356,372,54,white);r.text(45,367,"LAST MATCH",muted,1.05f);if(s.last.home>=0)r.text(45,389,std::string(clubs[s.last.home].code)+" "+num(s.last.hg)+" X "+num(s.last.ag)+" "+clubs[s.last.away].code+"  RATING "+num(s.last.rating10/10)+"."+num(s.last.rating10%10),ink,1.3f);else r.text(45,389,"YOUR FIRST CHAPTER IS WAITING.",ink,1.1f);
    r.rect(420,115,188,295,white);r.text(435,132,"YOUR PLAYER",muted,1.1f);r.rect(476,163,70,55,green);r.rect(458,163,18,23,green);r.rect(546,163,18,23,green);r.text(496,178,"27",lime,3);r.text(435,237,s.name,ink,1.35f,20);r.text(435,258,clubs[s.team].name,muted,1.1f,23);
    r.text(435,282,"ENERGY "+num(s.energy)+"%",muted,1.1f);bar(r,435,297,155,s.energy,s.energy<25?red:green);r.text(435,314,"MORALE "+num(s.morale)+"%",muted,1.1f);bar(r,435,329,155,s.morale);r.text(435,346,"XP "+num(s.xp)+" / 100",muted,1.1f);bar(r,435,361,155,s.xp);r.text(435,385,euro(s.salary)+" / WK.",green,1.1f);
   }else if(tab==1){
    r.text(32,117,"TALENT NEEDS TRAINING TOO.",ink,2);r.text(32,144,s.trained?"PREPARATION COMPLETE FOR THIS MATCHDAY.":"ONE ACTIVITY PER MATCHDAY. CHOOSE YOUR FOCUS.",muted,1.2f);
    const char*names[]={"SHOOTING + DRIBBLING","PASSING + DRIBBLING","PACE + PHYSICAL","RECOVER ENERGY"};const char*effects[]={"+1 TO TWO ATTRIBUTES / -16 ENERGY","+1 TO TWO ATTRIBUTES / -16 ENERGY","+1 TO TWO ATTRIBUTES / -16 ENERGY","+32 ENERGY / +3 MORALE"};
    for(int j=0;j<4;j++){r.rect(32,175+j*55,360,47,trainSelection==j?lime:white);r.text(45,184+j*55,names[j],green,1.3f);r.text(45,204+j*55,effects[j],muted,1);}
    for(int j=0;j<6;j++){r.text(416,180+j*31,attrs[j],muted,1.1f);r.text(560,180+j*31,num(s.attributes[j]),ink,1.3f);bar(r,416,195+j*31,174,s.attributes[j]);}r.text(32,412,"UP/DOWN: SELECT   A: TRAIN",green,1.15f);
   }else if(tab==2){
    r.text(32,116,"BERNAS LEAGUE",ink,2);r.text(372,122,"MATCHDAY "+num(s.round)+" / 14",muted,1.2f);r.rect(32,151,576,26,green);r.text(44,159,"#  CLUB",white,1.1f);r.text(362,159,"P    W    D    L    GD   PTS",white,1.1f);
    auto order=ranking(s);for(int j=0;j<8;j++){int id=order[j],y=183+j*27;auto t=s.table[id];r.rect(32,y-2,576,25,id==s.team?lime:white);r.text(44,y+5,num(j+1)+"  "+clubs[id].name,ink,1.2f);int values[]={t.played,t.w,t.d,t.l,t.gf-t.ga,t.pts()};for(int n=0;n<6;n++)r.text(363+n*40,y+5,num(values[n]),ink,1.1f);}r.text(32,412,"FICTIONAL LEAGUE / POINTS, GD, GOALS, ENTRY ORDER.",muted,1.1f);
   }else if(tab==3){
    r.text(32,116,"ONE SHIRT. NEW DREAMS.",ink,2);r.text(32,147,"REPUTATION "+num(s.reputation)+"/100 | "+euro(s.salary)+" / WEEK",muted,1.2f);r.rect(32,185,576,170,white);
    int target=s.pending>=0?s.pending:s.offer;
    if(target>=0){r.text(50,205,s.pending>=0?"YOUR NEXT CLUB":"OFFER RECEIVED",muted,1.3f);r.text(50,237,clubs[target].name,green,2.1f);r.text(50,275,euro(proposedSalary(s,target))+" / WEEK*",ink,1.5f);r.text(50,313,s.pending>=0?"CONTRACT SIGNED. JOINING NEXT SEASON.":"A: ACCEPT FOR NEXT SEASON",green,1.2f);}
    else{r.text(50,213,"THE MARKET IS WATCHING.",green,1.8f);r.text(50,253,"SCOUTS MAY ARRIVE AFTER MATCHDAYS 4, 8 AND 12.",muted,1.15f);r.text(50,286,"YOUR LEVEL DETERMINES WHICH CLUBS ARE INTERESTED.",muted,1.1f);}
    r.text(32,380,"FINISH THIS SEASON WITH YOUR CURRENT CLUB.",muted,1.1f);r.text(32,402,"*FINAL WAGE: REPUTATION AT THE NEW SEASON START.",muted,1.1f);
   }else if(tab==4){
    r.text(32,116,"YOUR LEGACY",ink,2);r.text(32,150,"MATCHES "+num(s.totalMatches)+"  GOALS "+num(s.totalGoals)+"  ASSIST. "+num(s.totalAssists)+"  TITLES "+num(s.titles),green,1.2f);
    if(s.history.empty()){r.rect(32,191,576,132,white);r.text(48,218,"EVERY LEGEND HAS A BEGINNING.",green,1.8f);r.text(48,259,"COMPLETE THE SEASON AND START THE NEXT ONE.",muted,1.2f);}
    else for(size_t j=0;j<s.history.size();j++){auto h=s.history[j];int y=183+int(j)*27;r.rect(32,y-2,576,25,white);r.text(42,y+5,num(h.year),green,1.1f);r.text(98,y+5,clubs[h.team].name,ink,1.1f);r.text(318,y+5,"G "+num(h.goals)+"  A "+num(h.assists)+"  PLACE "+num(h.rank),muted,1.1f);}
    r.text(32,412,"CAREER EARNINGS: "+euro(s.cash),green,1.1f);
   }else{
    r.text(32,116,"HOW TO PLAY",ink,2);const char*help[]={"D-PAD LEFT/RIGHT: SWITCH TAB","D-PAD UP/DOWN: SELECT","A: CONFIRM / SIMULATE / TRAIN","B: BACK TO CAREER",
#ifdef PLATFORM_WII
    "+: SAVE  -: LOAD  HOME: EXIT",
#else
    "START: SAVE  X: LOAD  Z: EXIT",
#endif
    "TRAIN, PLAY AND IMPROVE. 100 XP = ATTRIBUTE GAIN.","TRANSFERS TAKE EFFECT NEXT SEASON.","SAVE: BERNAS-CAREER.SAV AT THE FAT ROOT.","ORDINARY MEMORY CARDS ARE NOT SUPPORTED.","A: MAIN MENU / NEW CAREER"};for(int j=0;j<10;j++)r.text(32,163+j*25,help[j],j==9?green:muted,1.15f);
   }
  }
  r.rect(0,439,640,41,green);r.text(32,453,messageTimer>0?message:"LEFT/RIGHT: TABS | A: CONFIRM | B: CAREER",messageTimer>0?lime:white,1.1f,87);if(messageTimer>0)messageTimer--;r.finish();
 }
 if(screen==3)writeSave(s);return 0;
}
