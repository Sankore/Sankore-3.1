function delAllBubble(){
	for (var i = 0; i < nbPin;i++){
		var bubble = document.getElementById('bulle'+i);	
		if(bubble != undefined ){			
			bubble.parentNode.removeChild(bubble);
		}
	}
}
function switchToVisionneur(){
	var visionButton = document.getElementById('but');
	visionButton.src="images/pen.png";
	visionButton.style.height = 22;
	visionButton.style.width = 22;
	visionButton.setAttribute("onClick","switchToEditor()");
	for (var i = 0; i < nbPin;i++){
				var pin = document.getElementById('pin'+i);
				if (pin != undefined ){
					$("#pin"+i).draggable( "option", "disabled", true );
				} 
			}
	delAllBubble();
	isEditor = false;
}
function switchToEditor(){
	var visionButton = document.getElementById('but');
	visionButton.src="images/pointing_finger.png"
	visionButton.setAttribute("onClick","switchToVisionneur()");
	visionButton.style.height = 22;
	visionButton.style.width = 22;
	for (var i = 0; i < nbPin;i++){
				var pin = document.getElementById('pin'+i);
				if (pin != undefined ){
					$("#pin"+i).draggable( "option", "disabled", false );
				} 
			}
	delAllBubble();
	isEditor = true;
}
/** Create the widget is it's in Open-Sankore */
function createWindow(){

		if(window.sankore){
			if(sankore.preference("pictoNoteData","")){
				var importArray = jQuery.parseJSON(sankore.preference("pictoNoteData",""));
				var importstring = jQuery.parseJSON(sankore.preference("pictoNoteUrl",""));
				isEditor = false;
				creerpicto(true);
				switchToVisionneur();
				pins = importArray;
				imagePath = importstring;
				nbPin = pins.length;		
				resizing();
				inter();
			}
		}
}
/** Get previous presentation information if they exist*/
function inter (){
	var tex = document.getElementById('inpurl');
	if (imagePath==""){
		imagePath = tex.value;
		sankore.setPreference("pictoNoteUrl", JSON.stringify(imagePath));
	} else if(window.sankore){
			if(sankore.preference("pictoNoteData","")){
			var importArray = jQuery.parseJSON(sankore.preference("pictoNoteData",""));
			isEditor = false;
			creerpicto(true);
			switchToVisionneur();
			pins = importArray;
			nbPin = pins.length;		
			resizing();

		} else {
			creerpicto();	
		}
	}

}

/** Called when help button is clicked, to display help pannel*/
function displayHelp(){
	var hb = document.getElementById('helpButton');
	if (helping){
		hb.src = "images/helpButton.png";
		helping = false;
		$('#divHelp').css('display','none');
		if(window.uniboard)
			{
				window.uniboard.setPreference("width", 200);
				window.uniboard.setPreference("height", 200);
			}
		$("#image").draggable( "option", "disabled", false );
		
	} else {
		hb.src = "images/helpButtonPushed.png";
		helping = true;
		$('#divHelp').css('display','inline');
	if(window.uniboard)
		{
			window.uniboard.setPreference("width", 400);
			window.uniboard.setPreference("height", 400);
		}
	}
	
}
/** Create the widget*/
function creerpicto(isNew){	
	
	var visionButton =  document.createElement('img');
	visionButton.style.height = 22;
	visionButton.style.width = 22;
	visionButton.src = "images/pointing_finger.png";
	visionButton.setAttribute("onClick","switchToVisionneur()");
	visionButton.id = "but";	
	visionButton.style.display = "inline";
	visionButton.style.position = "absolute";
	
	var helpButton =  document.createElement('img');
	helpButton.src = "images/helpButton.png";
	helpButton.setAttribute("onClick","displayHelp()");
	helpButton.id = "helpButton";
	helpButton.style.display = "inline";
	helpButton.style.position = "absolute";
	helpButton.style.zIndex = 7;
	
	var appli = document.getElementById('appli');
	appli.innerHTML = "";
	appli.appendChild(visionButton);
	appli.appendChild(helpButton);
	var image = document.createElement('img');
	image.src = imagePath;
	image.style.top = 0;
	image.style.left = 0;

	image.id = "image";
	image.style.zIndex = 1;
	image.setAttribute("onClick","addPin(event)");
	appli.appendChild(image);
	
	image.onload = function(){
		height = image.offsetHeight;
		width = image.offsetWidth;
		if (height > width){
			image.style.height = "100%";
		}else{
			image.style.width = "100%";
		}
		if(isNew){
			for (var i = 0; i < nbPin;i++){
				createPin(i);
			}
		}
		visionButton.style.left= image.offsetLeft;
		visionButton.style.bottom= document.body.clientHeight-image.offsetHeight;
		helpButton.style.left= visionButton.offsetLeft+image.offsetWidth - 22;
		helpButton.style.bottom=  document.body.clientHeight-image.offsetHeight;
		resizing();
	}
	
	
		
}

