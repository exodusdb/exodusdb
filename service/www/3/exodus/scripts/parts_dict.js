async function dict_PARTS() {

 var dict=[]//of dictrecs
 var din=-1
 var di
 
 di=dict[++din]=dictrec('PART_CODE','F',0)
 
 di=dict[++din]=dictrec('PART_NAME','F',1)
 exodus_dict_text(di)
 di.required=true
 
 di=dict[++din]=dictrec('SUB_PART_CODE','F',2)
 di.filename='PARTS'
 di.unique=true
 di.validation=async function () {

  if (gvalue==gkey) return await exodusinvalid('Sorry, parts cannot be made of each other')
 }
 
 di=dict[++din]=dictrec('SUB_PART_NAME','S','','',1)
 exodus_dict_text(di)
 di.functioncode='return await this.xlate("SUB_PART_CODE", "PARTS",1,"X")'
 
 di=dict[++din]=dictrec('QUANTITY','F',3,'',1)
 exodus_dict_integer(di)
 di.required=true
 
 return dict
 
}