/* Pin number counter */
var nbPin = 0;

/** Deleting a Pin and its information*/
function onDel(nbPin){
	var bubble = document.getElementById('bulle'+nbPin);
	var pin = document.getElementById('pin'+nbPin);
	if (pin == undefined){
		var pin = document.getElementById('pinr'+nbPin);
	}
	bubble.parentNode.removeChild(bubble);
	pin.parentNode.removeChild(pin);
	pins[nbPin][0] = "N";
}

/** Saving Pins' informations*/
function onValidate(nbPin){
	var inp = document.getElementById("inp"+nbPin);
	pins[nbPin][2] = inp.value;
	sankore.setPreference("pictoNoteData", JSON.stringify(pins));
	pinClick(nbPin);
}

/** Change Pin's Color */
function changeColor(numPin,color){
	var pin = document.getElementById("pin"+numPin);
	if (!pin){
		var pinr = document.getElementById("pinr"+numPin);
		pinr.style.backgroundImage="url(images/pin-"+color+"-rev.png)";
		
	}else {
		pin.style.backgroundImage="url(images/pin-"+color+".png)";
	}
	pins[numPin][3] = color;
	var inp = document.getElementById("bulle"+numPin);
	var divBub = document.getElementById("bubbleDiv"+numPin);
	divBub.style.border = "solid 2px #"+tabColor[pins[numPin][3]];; 
	divBub.style.backgroundColor = "#"+tabColor[pins[numPin][3]];
}

/** Make Pin's text grow */
function growText(numPin){	
	pins[numPin][5]++;
	var input = document.getElementById("inp"+numPin);
	input.style.fontSize=(pins[numPin][5]);
	
}

/** Make Pin's text shrink */
function shrinkText(numPin){	
	pins[numPin][5]--;
	var input = document.getElementById("inp"+numPin);
	input.style.fontSize=(pins[numPin][5]);
}

/** Create and place the display to edit or show Pins' information*/
function createBubble(nbPin){
	/* Create the pin's edition pannel */
	if (isEditor){
		var div = document.createElement('div');
		var content = document.createElement('div');
		
		var tableTxt = document.createElement('table');
		var firstRowTT = document.createElement('tr');
		var secRowTT = document.createElement('tr');
		// var colblanc = document.createElement('td');
		var colText = document.createElement('td');
		var colTools = document.createElement('span');
		// var colBlanc3 = document.createElement('td');
		
		firstRowTT.appendChild(colText);
		tableTxt.appendChild(firstRowTT);
		
		var tableColor = document.createElement('span');
		// var colBlanc1 = document.createElement('span');
		// var colBlanc2 = document.createElement('span');
		var colBlack = document.createElement('span');
		var colRed   = document.createElement('span');
		var colBlue  = document.createElement('span');
		var colGreen = document.createElement('span');
		
		
		var black = document.createElement('img');
		var red = document.createElement('img');
		var blue = document.createElement('img');
		var green = document.createElement('img');
		
		black.src = "images/black.png";
		red  .src = "images/red.png";
		blue .src = "images/blue.png";
		green.src = "images/green.png";
		
		colTools.innerHTML=" ";
		// colBlanc1.innerHTML=" ";
		// colBlanc2.innerHTML=" ";
		// colBlanc3.innerHTML=" ";
		// colblanc.innerHTML=" ";
				
		colBlack.appendChild(black);
		colRed.appendChild(red);
		colBlue.appendChild(blue);
		colGreen.appendChild(green);
		
		var textSizeBigger =  document.createElement('img');
		var textSizer =  document.createElement('span');
		var textSizeSmaller =  document.createElement('img');
		
		tableColor.appendChild(textSizer);
		tableColor.appendChild(colBlack);
		tableColor.appendChild(colRed);
		tableColor.appendChild(colBlue);
		tableColor.appendChild(colGreen);
		tableColor.appendChild(colTools);
		
		var input = document.createElement('textarea');
		input.rows= 2;
		input.cols= 30;
		input.id="inp"+nbPin;
		input.style.fontSize = pins[nbPin][5];
		input.style.minWidth = 200;
		
		// delete button
		var delImage = document.createElement('img');
		delImage.src="images/delete.png";
		delImage.setAttribute("onClick","onDel("+nbPin+")");
		delImage.style.borderRadius = "5px";
		delImage.style.border = "solid 1px #838786";
		delImage.style.backgroundColor = "#B6BAB9";
		delImage.height =17;
		
		// validate button
		var valImage = document.createElement('img');
		valImage.src="images/validate.png";
		valImage.setAttribute("onClick","onValidate("+nbPin+")");
		valImage.style.borderRadius = "5px";
		valImage.style.border = "solid 1px #838786";
		valImage.style.backgroundColor = "#B6BAB9";
		valImage.height =17;
		
		black.setAttribute("onClick","changeColor("+nbPin+",'black')");
		red  .setAttribute("onClick","changeColor("+nbPin+",'red')");
		blue .setAttribute("onClick","changeColor("+nbPin+",'blue')");
		green.setAttribute("onClick","changeColor("+nbPin+",'green')");
		
		tableTxt.style.width="100%";
		tableColor.style.width="80%";
		colText.style.width="80%";
		
		textSizeBigger.src = "images/aPlus.png";			
		textSizeBigger.setAttribute("onClick","growText("+nbPin+")");
		textSizeBigger.height = 19;
		textSizeBigger.style.marginLeft = 1;
		textSizeBigger.style.marginRight = 4;
		textSizeSmaller.src = "images/aMoins.png";		
		textSizeSmaller.setAttribute("onClick","shrinkText("+nbPin+")");
		textSizeSmaller.height = 19;
		
		
		textSizer.appendChild(textSizeSmaller);
		textSizer.appendChild(textSizeBigger);
		
		input.className="img_block";
		input.style="text-align: center;";
		input.setAttribute("ondragenter","return false;");
		input.setAttribute("ondragleave",'$(this).css("background-color","white"); return false;');
		input.setAttribute("ondragover",'$(this).css("background-color","#ccc"); return false;');
		input.setAttribute("ondrop",'$(this).css("background-color","white"); return onDropTarget(this,event,'+nbPin+');');
		
		colText.appendChild(input);
		colTools.appendChild(valImage);
		colTools.appendChild(delImage);
		
		tableTxt.style.borderRadius = "10px";
		tableTxt.style.border = "solid 1px #"+tabColor[pins[nbPin][3]];
		tableTxt.style.backgroundColor = "#"+tabColor[pins[nbPin][3]];
		
		tableTxt.id="bubbleDiv"+nbPin;
		
		div.appendChild(tableTxt);
		
		content.style.textAlign="left";
		
		div.appendChild(tableColor);
		div.style.display = "inline";
		div.style.position = "absolute";
		
		content.style.display = "inline";
		content.style.position = "absolute";		
		content.appendChild(div);
		
		
		return content;
	}
	/* Create the pin's information display pannel */
	else {
		var div = document.createElement('div');
		var content = document.createElement('div');
		
		var text = document.createElement('div');
		text.id="text"+nbPin;
		text.innerHTML = " ";
		text.style.minWidth="100";
		
		div.appendChild(text);
		div.style.display = "inline";
		div.style.position = "absolute";
		
		div.style.borderRadius = "10px";
		div.id="bubbleDiv"+nbPin;
		div.style.border = "solid 2px #"+tabColor[pins[nbPin][3]];
		div.style.backgroundColor = "#"+tabColor[pins[nbPin][3]];
		div.style.fontSize = pins[nbPin][5];
		if (pins[nbPin][3] == "black"){
			
			div.style.color="#FFFFFF";
		}		
		div.style.textAlign = "center";
		
		content.style.display = "inline";
		content.style.position = "absolute";
		content.appendChild(div);
		
		
		return content;
	}
}

/** Function called when a pin is clicked, to create or delete his information pannel */
function pinClick(nbPin){
		var pin = document.getElementById('pin'+nbPin);
		var im = document.getElementById("image");
		var isReverse = !(pin !=undefined);
		var bubble = document.getElementById('bulle'+nbPin);	
		if(bubble == undefined){
			var image = createBubble(nbPin);
			image.id = "bulle"+nbPin;
			image.className  = "bulle";
			image.style.zIndex = 2;		
			appli.appendChild(image);
			
			var inp = document.getElementById("inp"+nbPin);		
			var text = document.getElementById("text"+nbPin);		
			if (inp!= undefined){			
				inp.value = pins[nbPin][2];
				inp.focus();
			}else if (text != undefined){
				text.innerHTML = pins[nbPin][2];	
				for(var i = 0; i< text.childNodes.length; i++){
					if(text.hasChildNodes()==true){
						 text.childNodes[i].onload = function (){resizing()};
					}
				 }	
			}
			// Bubble positioning
			if(image != undefined){		
				var bd = document.getElementById("bubbleDiv"+nbPin);	
				var image1 = document.getElementById('image');
				if (isReverse){
					var bublleStartX = Math.round(pins[nbPin][0]*image1.width)-(bd.offsetWidth+offsetPinX)+(im.offsetLeft);
					var bublleStartY = Math.round(pins[nbPin][1]*image1.height)-(bd.offsetHeight+offsetPinRevY);
				}
				else{
					var bublleStartX = Math.round(pins[nbPin][0]*image1.width)-(bd.offsetWidth+offsetPinX)+(im.offsetLeft);
					var bublleStartY = Math.round(pins[nbPin][1]*image1.height)-(bd.offsetHeight+offsetPinY);
				}
					
				if (bublleStartX > 0 ){
					image.style.textAlign="left";
					if (bublleStartY > 0 ){
					}else {
						if (isReverse){
							bublleStartY = bublleStartY + bd.offsetHeight+ offsetPinRevY;
							
						} else {
							bublleStartY = bublleStartY + bd.offsetHeight+offsetPinY;
						
						}
					}
				} else{
					image.style.textAlign="right";
					bublleStartX = bublleStartX + bd.offsetWidth+offsetPinX;
					if (bublleStartY > 0 ){			
					}else {
						if (isReverse){
							bublleStartY = bublleStartY +  bd.offsetHeight+ offsetPinRevY;
						
						} else {
							bublleStartY = bublleStartY +  bd.offsetHeight+offsetPinY;
						
						}
					}
				}
				image.style.left =bublleStartX;
				image.style.top =bublleStartY;
			}
			
		}
		// If the bubble already exist, delete the bubble
		else{
			bubble.parentNode.removeChild(bubble);
		}
}

/** Called when the image is clicked on to add a pin */
function addPin(event){	
	if (isEditor){
		var appli = document.getElementById('appli');
		var image = document.getElementById('image');	
		// get click position
		pos_x = event.offsetX?(event.offsetX):event.pageX-document.getElementById("pointer_div").offsetLeft;
		pos_y = event.offsetY?(event.offsetY):event.pageY-document.getElementById("pointer_div").offsetTop;
		// pins <=> [horizontal ratio, vertical ratio, text, color, orientation, textsize]
		pins [nbPin]= [pos_x/image.width,pos_y/image.height,"","black","nonOriente",12];
		
		var pin = document.createElement('div');
		pin.style.display = "inline";
		pin.style.position = "absolute";
		pin.style.zIndex = 3;
		pin.setAttribute("onClick","pinClick("+nbPin+");");
		//pin.setAttribute("onDrag","");
		pin.setAttribute("name", nbPin);
		//adding pin to the window
		appli.appendChild(pin);
		
		
		// if click position is too high, the pin icon is upside-down
		if(pos_y <offsetPinY){
			pin.id = "pinr"+nbPin;
			pin.style.backgroundImage="url(images/pin-black-rev.png)";
			pin.style.width = 25
			pin.style.height = 25;
			pin.style.top = pos_y - offsetPinRevY;
			pin.style.left = pos_x - offsetPinRevX +(image.offsetLeft);
		}else{
			pin.id = "pin"+nbPin;
			pin.style.backgroundImage = "url(images/pin-black.png)";
			pin.style.width = 25;
			pin.style.height = 25;
			pin.style.top = pos_y - offsetPinY;
			pin.style.left = pos_x - offsetPinX +(image.offsetLeft);
		}	
		// on drop change position ration
		if(pin.id.charAt(3)!='r'){
			$(pin).draggable({ containment: "#image", cursorAt:{top:offsetPinY,left:offsetPinX},
					stop: function() {
						var pinNum = parseInt(this.getAttribute("name"));
						var stopPos = $(this).position();
						var image = document.getElementById('image');
						pins[pinNum][0] = (stopPos.left+offsetPinX-image.offsetLeft)/image.width;
						pins[pinNum][1] = (stopPos.top+offsetPinY)/image.height;
						resizing();				
					}
				});
		}else{
			$(pin).draggable({ containment: "#image", cursorAt:{top:offsetPinRevY,left:offsetPinRevX},
					stop: function() {
						var pinNum = parseInt(this.getAttribute("name"));
						var stopPos = $(this).position();
						var image = document.getElementById('image');
						pins[pinNum][0] = (stopPos.left+offsetPinRevX-image.offsetLeft)/image.width;
						pins[pinNum][1] = (stopPos.top+offsetPinRevY)/image.height;
						resizing();				
					}
				});
		
		}
		// updating the number of pin
		pinClick(nbPin);
		nbPin ++;
	}
}
/** Pin's creation, called when the widget is loaded from a previous presentation*/
function createPin(numPin){
	if (pins[numPin][0] != "N"){			
			var appli = document.getElementById('appli');
			var image = document.getElementById('image');	
			// get pin's position
			pos_x =  Math.round(pins[numPin][0]*image.width)-offsetPinX;
			pos_y =  Math.round(pins[numPin][1]*image.height)-offsetPinY;			
			
			var pin = document.createElement('img');
			pin.style.display = "inline";
			pin.style.position = "absolute";
			pin.style.zIndex = 3;
			pin.setAttribute("onClick","pinClick("+numPin+")");
			pin.setAttribute("class", "pin");
			//adding pin to the window
			appli.appendChild(pin);
		
			// if click position is too high, the pin icon is upside-down
			if(pos_y <offsetPinY){
				pin.id = "pinr"+numPin;
				pin.src = "images/pin-"+pins[numPin][3]+"-rev.png";
				pin.style.top = pos_y - offsetPinRevY;
				pin.style.left = pos_x - offsetPinRevX;
			}else{
				pin.id = "pin"+numPin;
				pin.src = "images/pin-"+pins[numPin][3]+".png";
				pin.style.top = pos_y - offsetPinY;
				pin.style.left = pos_x - offsetPinX;
			}
			
			// set draggable properties to the pin
			if(pin.id.charAt(3)!='r'){
				$(pin).draggable({ containment: "#image", cursor: "cross",cursorAt:{top:offsetPinY,left:offsetPinX},
					stop: function() {
						var pinNum = parseInt(this.getAttribute("name"));
						var stopPos = $(this).position();
						var image = document.getElementById('image');
						pins[pinNum][0] = (stopPos.left+offsetPinX)/image.width;
						pins[pinNum][1] = (stopPos.top+offsetPinY)/image.height;
						resizing();				
					}
				});
			}else{
			$(pin).draggable({ containment: "#image", cursorAt:{top:offsetPinRevY,left:offsetPinRevX},
					stop: function() {
						var pinNum = parseInt(this.getAttribute("name"));
						var stopPos = $(this).position();
						var image = document.getElementById('image');
						pins[pinNum][0] = (stopPos.left+offsetPinRevX)/image.width;
						pins[pinNum][1] = (stopPos.top+offsetPinRevY)/image.height;
						resizing();				
					}
				});
		
		}
			// updating the number of pin
			pinClick(numPin);
			numPin ++;
	}
}