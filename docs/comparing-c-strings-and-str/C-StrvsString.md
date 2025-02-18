Interestingly, in the use case described above (which is what we do for RP) there wasn't any difference between using c-strings and String from arduino 

```
Time spent creating storeStr with String: 0
Asensorvals pt1=32.13, pt2=43.15, pt3=231.78, pt4=0, pt5=0, pt6=0, lc1=90129, lc2=0
Time spent printing to Serial: 123
---------------------------------
Time spent creating storeStr with c-str: 0
Asensorvals pt1=32.13,pt2=43.15,pt3=231.78,pt4=0,pt5=0,pt6=0,lc1=0.000000,lc2=1.690125Z
Time spent printing to Serial: 122
---------------------------------
Time spent creating storeStr with String: 0
Asensorvals pt1=32.13, pt2=43.15, pt3=231.78, pt4=0, pt5=0, pt6=0, lc1=90129, lc2=0
Time spent printing to Serial: 123
---------------------------------
Time spent creating storeStr with c-str: 0
Asensorvals pt1=32.13,pt2=43.15,pt3=231.78,pt4=0,pt5=0,pt6=0,lc1=0.000000,lc2=1.690125Z
Time spent printing to Serial: 122
---------------------------------
```

Only when cranking up the number of readings to 70 (an amount that was not practical for our use cases), was there a even difference in the C-str vs String
```
Time spent creating storeStr with String: 3
Asensorvals pt1=32.13, pt2=43.15, pt3=231.78, pt4=0.00, pt5=0.00, pt6=0.00, pt7=0.00, pt8=0.00, pt9=0.00, pt10=0.00, pt11=0.00, pt12=0.00, pt13=0.00, pt14=0.00, pt15=0.00, pt16=0.00, pt17=0.00, pt18=0.00, pt19=0.00, pt20=0.00, pt21=0.00, pt22=0.00, pt23=0.00, pt24=0.00, pt25=0.00, pt26=0.00, pt27=0.00, pt28=0.00, pt29=0.00, pt30=0.00, pt31=0.00, pt32=0.00, pt33=0.00, pt34=0.00, pt35=0.00, pt36=0.00, pt37=0.00, pt38=0.00, pt39=0.00, pt40=0.00, pt41=0.00, pt42=0.00, pt43=0.00, pt44=0.00, pt45=0.00, pt46=0.00, pt47=0.00, pt48=0.00, pt49=0.00, pt50=0.00, pt51=0.00, pt52=0.00, pt53=0.00, pt54=0.00, pt55=0.00, pt56=0.00, pt57=0.00, pt58=0.00, pt59=0.00, pt60=0.00, pt61=0.00, pt62=0.00, pt63=0.00, pt64=0.00, pt65=0.00, pt66=0.00, pt67=0.00, pt68=0.00, pt69=0.00, pt70=0.00, pt4=0, pt5=0, pt6=0, lc1=90129, lc2=0
Time spent printing to Serial: 859
---------------------------------
Time spent creating storeStr with c-str: 2
Asensorvals pt1=32.13,pt2=43.15,pt3=231.78,pt4=0.00,pt5=0.00,pt6=0.00,pt7=0.00,pt8=0.00,pt9=0.00,pt10=0.00,pt11=0.00,pt12=0.00,pt13=0.00,pt14=0.00,pt15=0.00,pt16=0.00,pt17=0.00,pt18=0.00,pt19=0.00,pt20=0.00,pt21=0.00,pt22=0.00,pt23=0.00,pt24=0.00,pt25=0.00,pt26=0.00,pt27=0.00,pt28=0.00,pt29=0.00,pt30=0.00,pt31=0.00,pt32=0.00,pt33=0.00,pt34=0.00,pt35=0.00,pt36=0.00,pt37=0.00,pt38=0.00,pt39=0.00,pt40=0.00,pt41=0.00,pt42=0.00,pt43=0.00,pt44=0.00,pt45=0.00,pt46=0.00,pt47=0.00,pt48=0.00,pt49=0.00,pt50=0.00,pt51=0.00,pt52=0.00,pt53=0.00,pt54=0.00,pt55=0.00,pt56=0.00,pt57=0.00,pt58=0.00,pt59=0.00,pt60=0.00,pt61=0.00,pt62=0.00,pt63=0.00,pt64=0.00,pt65=0.00,pt66=0.00,pt67=0.00,pt68=0.00,pt69=0.00,pt70=0.00,pt4=0,pt5=0,pt6=0,lc1=0.000000,lc2=1.692032Z
Time spent printing to Serial: 738
---------------------------------
```

The conclusion is that as much as String() is hated, there is really no need to go replacing it for our needs.
Also interesting fact I have observed is that the time taken to print to Serial is affected solely by the size of the buffer that you want to print.