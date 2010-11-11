// JScript File

function neosys_val_font()
{
 if (gvalue.toUpperCase()=='DEFAULT') gvalue=''
 return true
}

function neosys_val_color()
{
 if (gvalue.toUpperCase()=='DEFAULT') gvalue=''
 return true
}

function neosys_dict_font(di)
{

 di.lowercase=true;
 di.allowcursor=true
 
 //get fonts
 var tt=''
 if (!di.required) tt+='Default;Default:'
 tt=neosys_get_fonts(tt)
 
 di.conversion=tt

 di.popup='neosys_pop_font('
 if (di.required) di.popup+='true'
 di.popup+=')'

 return

}

function neosys_pop_font(required,many)
{

 //get colors
 var tt=''
 if (!required) tt+='Default;Default:'
 tt=neosys_get_fonts(tt)
 
 tt=tt.split(':')
 for (ii=tt.length-1;ii>=0;ii--)
 {
  var ttt=tt[ii].split(';')
  ttt[1]=''
  ttt[2]='<div width=100% style="font:'+ttt[0]+'">'+ttt[0]
  //exclude this since it still works without it and it triggers XML mode in neosysdecide2
  //ttt[2]+='</div>'
  tt[ii]=ttt.join(vm)
 }
 tt=tt.join(fm)
 
 return neosysdecide2('',tt,[[2,'']],0,'',many)
  
}

function neosys_dict_color(di)
{

 di.lowercase=true;
 di.allowcursor=true

 //get colors
 var tt=''
 if (!di.required) tt+=';Default:'
 tt=neosys_get_colors(tt)
 
 di.conversion=tt

 di.popup='neosys_pop_color('
 if (di.required) di.popup+='true'
 di.popup+=')'
 
 return
 
}

function neosys_pop_color(required,many)
{

 //get colors
 var tt=''
 if (!required) tt+=';Default:'
 tt=neosys_get_colors(tt)
 
 //use contents to show colors in popup
 tt=tt.split(':')
 for (ii=tt.length-1;ii>=0;ii--)
 {
  var ttt=tt[ii].split(';')
  ttt[2]='<div width=100% style="background-color:'+ttt[0]+'">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'//+ttt[1]
  //exclude this since it still works without it and it triggers XML mode in neosysdecide2
  //ttt[2]+='</div>'
  tt[ii]=ttt.join(vm)
 }
 tt=tt.join(fm)
 
 return neosysdecide2('',tt,[[2,''],[1,''],[0,'']],1,'',many)
 
}

function neosys_get_colors(tt)
{
if (!tt) tt=''
tt+='#000000;Black:'
tt+='#000080;Navy:'
tt+='#00008B;DarkBlue:'
tt+='#0000CD;MediumBlue:'
tt+='#0000FF;Blue:'
tt+='#006400;DarkGreen:'
tt+='#008000;Green:'
tt+='#008080;Teal:'
tt+='#008B8B;DarkCyan:'
tt+='#00BFFF;DeepSkyBlue:'
tt+='#00CED1;DarkTurquoise:'
tt+='#00FA9A;MediumSpringGreen:'
tt+='#00FF00;Lime:'
tt+='#00FF7F;SpringGreen:'
tt+='#00FFFF;Aqua:'
tt+='#00FFFF;Cyan:'
tt+='#191970;MidnightBlue:'
tt+='#1E90FF;DodgerBlue:'
tt+='#20B2AA;LightSeaGreen:'
tt+='#228B22;ForestGreen:'
tt+='#2E8B57;SeaGreen:'
tt+='#2F4F4F;DarkSlateGray:'
tt+='#2F4F4F;DarkSlateGrey:'
tt+='#32CD32;LimeGreen:'
tt+='#333399;NavyBlue:'
tt+='#3CB371;MediumSeaGreen:'
tt+='#40E0D0;Turquoise:'
tt+='#4169E1;RoyalBlue:'
tt+='#4682B4;SteelBlue:'
tt+='#483D8B;DarkSlateBlue:'
tt+='#48D1CC;MediumTurquoise:'
tt+='#4B0082;Indigo :'
tt+='#556B2F;DarkOliveGreen:'
tt+='#5F9EA0;CadetBlue:'
tt+='#6495ED;CornflowerBlue:'
tt+='#66CDAA;MediumAquaMarine:'
tt+='#696969;DimGray:'
tt+='#696969;DimGrey:'
tt+='#6A5ACD;SlateBlue:'
tt+='#6B8E23;OliveDrab:'
tt+='#708090;SlateGray:'
tt+='#708090;SlateGrey:'
tt+='#778899;LightSlateGray:'
tt+='#778899;LightSlateGrey:'
tt+='#7B68EE;MediumSlateBlue:'
tt+='#7CFC00;LawnGreen:'
tt+='#7FFF00;Chartreuse:'
tt+='#7FFFD4;Aquamarine:'
tt+='#800000;Maroon:'
tt+='#800080;Purple:'
tt+='#808000;Olive:'
tt+='#808080;Gray:'
tt+='#808080;Grey:'
tt+='#87CEEB;SkyBlue:'
tt+='#87CEFA;LightSkyBlue:'
tt+='#8A2BE2;BlueViolet:'
tt+='#8B0000;DarkRed:'
tt+='#8B008B;DarkMagenta:'
tt+='#8B4513;SaddleBrown:'
tt+='#8FBC8F;DarkSeaGreen:'
tt+='#90EE90;LightGreen:'
tt+='#9370D8;MediumPurple:'
tt+='#9400D3;DarkViolet:'
tt+='#98FB98;PaleGreen:'
tt+='#9932CC;DarkOrchid:'
tt+='#9ACD32;YellowGreen:'
tt+='#A0522D;Sienna:'
tt+='#A52A2A;Brown:'
tt+='#A9A9A9;DarkGray:'
tt+='#A9A9A9;DarkGrey:'
tt+='#ADD8E6;LightBlue:'
tt+='#ADFF2F;GreenYellow:'
tt+='#AFEEEE;PaleTurquoise:'
tt+='#B0C4DE;LightSteelBlue:'
tt+='#B0E0E6;PowderBlue:'
tt+='#B22222;FireBrick:'
tt+='#B8860B;DarkGoldenRod:'
tt+='#BA55D3;MediumOrchid:'
tt+='#BC8F8F;RosyBrown:'
tt+='#BDB76B;DarkKhaki:'
tt+='#C0C0C0;Silver:'
tt+='#C71585;MediumVioletRed:'
tt+='#CD5C5C;IndianRed :'
tt+='#CD853F;Peru:'
tt+='#D2691E;Chocolate:'
tt+='#D2B48C;Tan:'
tt+='#D3D3D3;LightGray:'
tt+='#D3D3D3;LightGrey:'
tt+='#D87093;PaleVioletRed:'
tt+='#D8BFD8;Thistle:'
tt+='#DA70D6;Orchid:'
tt+='#DAA520;GoldenRod:'
tt+='#DC143C;Crimson:'
tt+='#DCDCDC;Gainsboro:'
tt+='#DDA0DD;Plum:'
tt+='#DEB887;BurlyWood:'
tt+='#E0FFFF;LightCyan:'
tt+='#E6E6FA;Lavender:'
tt+='#E9967A;DarkSalmon:'
tt+='#EE82EE;Violet:'
tt+='#EEE8AA;PaleGoldenRod:'
tt+='#F08080;LightCoral:'
tt+='#F0E68C;Khaki:'
tt+='#F0F8FF;AliceBlue:'
tt+='#F0FFF0;HoneyDew:'
tt+='#F0FFFF;Azure:'
tt+='#F4A460;SandyBrown:'
tt+='#F5DEB3;Wheat:'
tt+='#F5F5DC;Beige:'
tt+='#F5F5F5;WhiteSmoke:'
tt+='#F5FFFA;MintCream:'
tt+='#F8F8FF;GhostWhite:'
tt+='#FA8072;Salmon:'
tt+='#FAEBD7;AntiqueWhite:'
tt+='#FAF0E6;Linen:'
tt+='#FAFAD2;LightGoldenRodYellow:'
tt+='#FDF5E6;OldLace:'
tt+='#FF0000;Red:'
tt+='#FF00FF;Fuchsia:'
tt+='#FF00FF;Magenta:'
tt+='#FF1493;DeepPink:'
tt+='#FF4500;OrangeRed:'
tt+='#FF6347;Tomato:'
tt+='#FF69B4;HotPink:'
tt+='#FF7F50;Coral:'
tt+='#FF8C00;Darkorange:'
tt+='#FFA07A;LightSalmon:'
tt+='#FFA500;Orange:'
tt+='#FFB6C1;LightPink:'
tt+='#FFC0CB;Pink:'
tt+='#FFD700;Gold:'
tt+='#FFDAB9;PeachPuff:'
tt+='#FFDEAD;NavajoWhite:'
tt+='#FFE4B5;Moccasin:'
tt+='#FFE4C4;Bisque:'
tt+='#FFE4E1;MistyRose:'
tt+='#FFEBCD;BlanchedAlmond:'
tt+='#FFEFD5;PapayaWhip:'
tt+='#FFF0F5;LavenderBlush:'
tt+='#FFF5EE;SeaShell:'
tt+='#FFF8DC;Cornsilk:'
tt+='#FFFACD;LemonChiffon:'
tt+='#FFFAF0;FloralWhite:'
tt+='#FFFAFA;Snow:'
tt+='#FFFF00;Yellow:'
tt+='#FFFFE0;LightYellow:'
tt+='#FFFFF0;Ivory:'
tt+='#FFFFFF;White:'

if (tt.slice(-1)==':') tt=tt.slice(0,-1)
if (tt.slice(0,1)==':') tt=tt.slice(1)

return tt
}

function neosys_get_fonts(tt)
{
if (!tt) tt=''

tt+='SANS SERIF,HELVETICA:'
tt+=' Arial:'
tt+=' Arial Black:'
tt+=' Arial Narrow:'
tt+=' Century Gothic:'
tt+=' Comic Sans MS:'
tt+=' Franklin Gothic Medium:'
tt+=' Haettenschweiler:'
tt+=' Impact:'
tt+=' Lucida Console:'
tt+=' Lucida Sans Unicode:'
tt+=' Microsoft Sans Serif:'
tt+=' MS Reference Sans Serif:'
tt+=' Nina:'
tt+=' Tahoma:'
tt+=' Trebuchet MS:'
tt+=' Verdana:'

tt+='SERIF,TIMES:'
tt+=' Book Antiqua:'
tt+=' Bookman Old Style:'
tt+=' Courier New:'
tt+=' Garamond:'
tt+=' Georgia:'
tt+=' Palatino Linotype:'
tt+=' Sylfaen:'
tt+=' Times New Roman:'
tt+=' Monotype Corsiva:'

if (tt.slice(-1)==':') tt=tt.slice(0,-1)
if (tt.slice(0,1)==':') tt=tt.slice(1)

return tt

}