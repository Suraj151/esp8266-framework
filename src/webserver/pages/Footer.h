#ifndef _EW_SERVER_FOOTER_HTML_H_
#define _EW_SERVER_FOOTER_HTML_H_

#include <Arduino.h>
#include <database/StoreStruct.h>

static const char EW_SERVER_FOOTER_HTML[] PROGMEM = "\
</div>\
<script>\
setTimeout(function(){\
document.getElementsByClassName('msg')[0].style.display='none';\
},4000);\
</script>\
</body>\
</html>";

static const char EW_SERVER_FOOTER_WITH_MONITOR_LISTEN_HTML[] PROGMEM = "\
</div>\
<script>\
var rq=new XMLHttpRequest();\
function rql(){\
var r=JSON.parse(this.responseText);\
var sv=document.getElementById('svga0');\
var ln = document.createElementNS('http://www.w3.org/2000/svg','line');\
if(r.r)location.href='/';\
ln.setAttribute('x1',r.x1);\
ln.setAttribute('y1',r.y1);\
ln.setAttribute('x2',r.x2);\
ln.setAttribute('y2',r.y2);\
ln.setAttribute('stroke','red');\
var nc=sv.childElementCount;\
if(nc>32){\
for(var i=3;i<nc;i++){\
var cn=sv.childNodes[i];\
cn.setAttribute('x1',cn.getAttribute('x1')-10);\
cn.setAttribute('x2',cn.getAttribute('x2')-10);\
}\
sv.removeChild(sv.childNodes[3]);\
}\
sv.appendChild(ln)\
}\
rq.addEventListener('load',rql);\
setInterval(function(){\
rq.open('GET','/listen-monitor');\
rq.send();\
},1000);\
</script>\
</body>\
</html>\
";

static const char EW_SERVER_FOOTER_WITH_CLIENT_LISTEN_HTML[] PROGMEM = "\
</div>\
<script>\
setTimeout(function(){\
document.getElementsByClassName('msg')[0].style.display='none';\
},4000);\
var rq=new XMLHttpRequest();\
function rql(){\
var r=JSON.parse(this.responseText);\
if(r.rpc){\
document.getElementsByClassName('cntnr')[0].innerHTML=r.bdy;\
}\
}\
rq.addEventListener('load',rql);\
setInterval(function(){\
rq.open('GET','/listen-client');\
rq.send();\
},15000);\
</script>\
</body>\
</html>";

#endif
