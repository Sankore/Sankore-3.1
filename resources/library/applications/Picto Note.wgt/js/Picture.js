/** Called everytime the window is resized */
function resizing(){	
	var image1 = document.getElementById('image');
	var isReverse;
	var appli = document.getElementById('appli');
	// Image resizing
	if(image1){
		var vRatio = image1.offsetWidth / document.body.clientWidth;
		var hRatio = image1.offsetHeight / document.body.clientHeight;
		if (vRatio < hRatio) {
			image1.style.width="";
			image1.style.height="100%";
		}else if (vRatio > hRatio) {
			image1.style.width="100%"
			image1.style.height="";
		}
	}
	// Button repositioning
	var helpButton = document.getElementById('helpButton');
	var viBut= document.getElementById('but');
	if(viBut){
		if(helpButton){
			helpButton.style.left= viBut.offsetLeft+image.offsetWidth - 22;
			helpButton.style.bottom= document.body.clientHeight-image.offsetHeight;
		}
		viBut.style.left= image1.offsetLeft;
		viBut.style.bottom= document.body.clientHeight-image.offsetHeight;
	}
	// Pins repositioning
	if(pins != ""){
		for (var i = 0; i<nbPin;i++){
			var pin = document.getElementById('pin'+i);					
			if(pin != undefined){		
				isReverse = false;
				pin.style.top = Math.round(pins[i][1]*image1.height)-offsetPinY;
				pin.style.left = Math.round(pins[i][0]*image1.width)-offsetPinX +(image.offsetLeft);
			}
			var pinr = document.getElementById('pinr'+i);
			if(pinr != undefined){			 
				isReverse = true;
				pinr.style.top = Math.round(pins[i][1]*image1.height)-offsetPinRevY;
				pinr.style.left = Math.round(pins[i][0]*image1.width)-offsetPinRevX +(image.offsetLeft);
			}		
			// Bubble repositioning
			var bubble = document.getElementById('bulle'+i);
			if(bubble != undefined){
			var bd = document.getElementById("bubbleDiv"+i);		
			if (isReverse){
				var bublleStartX = Math.round(pins[i][0]*image1.width)-(bd.offsetWidth+offsetPinX) +(image.offsetLeft);
				var bublleStartY = Math.round(pins[i][1]*image1.height)-(bd.offsetHeight+offsetPinRevY);
			}
			else{
				var bublleStartX = Math.round(pins[i][0]*image1.width)-(bd.offsetWidth+offsetPinX) +(image.offsetLeft);
				var bublleStartY = Math.round(pins[i][1]*image1.height)-(bd.offsetHeight+offsetPinY);
			}
				if (bublleStartX > 0 ){
					bubble.style.textAlign="left";
					//reDraw(i,"upLeft");
					if (bublleStartY > 0 ){
					}else {
						if (isReverse){
							bublleStartY = bublleStartY + bd.offsetHeight+ offsetPinRevY;
							
							//reDraw(i,"downLeft");
						} else {
							bublleStartY = bublleStartY + bd.offsetHeight+offsetPinY;
							//reDraw(i,"downLeft");
						}
					}
				} else{
					bubble.style.textAlign="right";
					// reDraw(i,"upRight");
					bublleStartX = bublleStartX + bd.offsetWidth+offsetPinX;
					if (bublleStartY > 0 ){			
					}else {
						if (isReverse){
							bublleStartY = bublleStartY +  bd.offsetHeight+ offsetPinRevY;
							// reDraw(i,"downRight");
						} else {
							bublleStartY = bublleStartY +  bd.offsetHeight+offsetPinY;
							// reDraw(i,"downRight");
						}
					}
				}
				bubble.style.top = bublleStartY;
				bubble.style.left = bublleStartX;
			}			
		}
	}
}
$(".img_block").attr("ondragover", "$(this).css(\"background-color\",\"#ccc\"); return false;");
//on lache l'image sur la "case"
$(".img_block").attr("ondrop", "$(this).css(\"background-color\",\"white\"); return onDropTarget(this,event);");

/** Open-Sankore D&D parser*/
function stringToXML(text){
    if (window.ActiveXObject){
        var doc=new ActiveXObject('Microsoft.XMLDOM');
        doc.async='false';
        doc.loadXML(text);
    } else {
        var parser=new DOMParser();
        doc=parser.parseFromString(text,'text/xml');
    }
    return doc;
}
/** Called when something is dropped on the widget*/
function onDropTarget(obj, event,numPin) {
    $(obj).find("img").remove();
    if (event.dataTransfer) {
        var format = "text/plain";
        var textData = event.dataTransfer.getData(format);
        if (!textData) {
            alert(":(");
        }
        textData = stringToXML(textData);
        var tmp = textData.getElementsByTagName("path")[0].firstChild.textContent;
        tmp = tmp.substr(1, tmp.length);
		var extention = tmp.split(".")[(tmp.split(".").length)-1];
		
		var imp = document.getElementById("imp");
		//create a diffente tag depending on what is dropped on the widget
		switch (extention) {
			case "mp3":
				var inp = document.getElementById("inp"+obj.id);
				if(inp){
					inp.value = inp.value + '<img src="'+tmp+'" width="70"/>';
					imp.parentNode.appendChild(img);
				}else{
					var inp = document.getElementById("inp"+numPin);
					inp.value = inp.value + "<audio controls=\"controls\"><source src=\""+tmp+"\" type=\"audio/"+extention+"\" /> </audio>";
				}
				break;
			case "jpg":
			case "png":
			case "jpeg":
			case "gif":
			case "bmp":
				// is no image has been selected the dropped element is the new image
				if (imagePath == ""){
					imagePath = tmp;
					sankore.setPreference("pictoNoteUrl", JSON.stringify(imagePath));
					inter();
				}else {
					var inp = document.getElementById("inp"+obj.id);
					
					if(inp){
						inp.value = inp.value + '<img src="'+tmp+'" width="70"/>';
						imp.parentNode.appendChild(img);
					}else{
						var inp = document.getElementById("inp"+numPin);
						inp.value = inp.value + '<img src="'+tmp+'" width="70"/>';
					}
				}
				
				
				
				imp.parentNode.removeChild(imp);
				break;
			default: 
				break;
		}
		
    }
    else {
        alert ("Your browser does not support the dataTransfer object.");
    }

    if (event.stopPropagation) {
        event.stopPropagation ();
    }
    else {
        event.cancelBubble = true;
    }
    return false;
}