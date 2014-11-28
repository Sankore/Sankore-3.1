/*	variables */

	var bInc = document.getElementsByClassName('inc');
	var bDec = document.getElementsByClassName('dec');
	
	var bAdd = document.getElementsByClassName('add');
	var bRemove = document.getElementsByClassName('remove');
	
	var vLastClass = 'bleu';

/*	function */	
	
	function increment(selectedNode){
		if(selectedNode.innerHTML<9){
			selectedNode.innerHTML++;
			return true;
		}else{
			var classType = selectedNode.parentNode.getElementsByClassName('type')[0];
			if(classType.innerHTML == 'U' || classType.innerHTML == 'D'){
				if(increment(selectedNode.parentNode.previousElementSibling.getElementsByTagName('span')[0])){
					selectedNode.innerHTML = 0;
					return true;
				}else{
					return false;
				}
			}else if(classType.innerHTML == 'C'){
				if( selectedNode.parentNode.parentNode.getAttribute('class').indexOf(vLastClass) == -1){
					if(increment(selectedNode.parentNode.parentNode.previousElementSibling.getElementsByClassName('unit')[2].getElementsByTagName('span')[0])){
						selectedNode.innerHTML = 0;
						return true;
					}else{
						return false;
					}
				}else{return false;}
			}
		}	
	}
	
	function decrement(selectedNode){
		if(selectedNode.innerHTML>0){
			selectedNode.innerHTML--;
			return true;
		}else{
			var classType = selectedNode.parentNode.getElementsByClassName('type')[0];
			if(classType.innerHTML == 'U' || classType.innerHTML == 'D'){
				if(decrement(selectedNode.parentNode.previousElementSibling.getElementsByTagName('span')[0])){
					selectedNode.innerHTML = 9;
					return true;
				}else{
					return false;
				}
			}else if(classType.innerHTML == 'C'){
				if( selectedNode.parentNode.parentNode.getAttribute('class').indexOf(vLastClass) == -1){
					if(decrement(selectedNode.parentNode.parentNode.previousElementSibling.getElementsByClassName('unit')[2].getElementsByTagName('span')[0])){
						selectedNode.innerHTML = 9;
						return true;
					}else{
						return false;
					}	
				}else{
					return false;
				}
			}
		}	
	}
	
	function addClass(){
		if(vLastClass.indexOf('jaune')== -1){				
			var actualClass = document.getElementsByClassName(vLastClass)[0];
			if(actualClass.className.indexOf(' last')>=0){ 
				actualClass.className = actualClass.className.replace(' last',''); 
			} 
			var classToAdd = actualClass.previousElementSibling;
			classToAdd.className = classToAdd.className + " last";
			classToAdd.style.display = "table-cell";
			vLastClass =classToAdd.getAttribute('class');
			return true;
		}else{
			return true;
		}	
	}
		
	function removeClass(){
		if(vLastClass.indexOf('bleu')== -1){
			var classToRemove = document.getElementsByClassName(vLastClass)[0];
			if(classToRemove.className.indexOf(' last')>=0){ 
				classToRemove.className = classToRemove.className.replace(' last',''); 
			} 
			classToRemove.style.display = "none";			
			var unitToInitiate = classToRemove.getElementsByClassName('unit');
			for(var i=0;i<unitToInitiate.length;i++){
				unitToInitiate[i].getElementsByTagName('span')[0].innerHTML = 0;
			}			
			var nextClass=classToRemove.nextElementSibling;
			vLastClass =nextClass.getAttribute('class');
			nextClass.className = nextClass.className + ' last'; 
			return true;
		}else{
			return true;
		}
	}
	
	function getNumber(){
		var myNumber = '';
		var unitCell = document.getElementsByClassName('unit');
		for(var i=0;i<unitCell.length;i++){
			myNumber += unitCell[i].getElementsByTagName('span')[0].innerHTML;
		}
		return myNumber;
	}
	
	function setNumber(savedNumber, lastClass){
		if(savedNumber){
			var unitCell = document.getElementsByClassName('unit');			
			for (var i=0; i<=11; i++) {
				unitCell[i].getElementsByTagName('span')[0].innerHTML = savedNumber.charAt(i);
			}
		}
		if(lastClass){		
			vLastClass = lastClass;
			var baseClass = document.getElementsByClassName(lastClass)[0];
			baseClass.style.display = "table-cell";
			baseClass.className = baseClass.className + " last";
			if(lastClass.indexOf('jaune')!= -1){
				document.getElementsByClassName('vert')[0].style.display = "table-cell";
				document.getElementsByClassName('orange')[0].style.display = "table-cell";
				return;
			}else if(lastClass.indexOf('vert')!= -1){
				document.getElementsByClassName('orange')[0].style.display = "table-cell";
				return;
			}
		}else{
			var actualClass = document.getElementsByClassName('bleu')[0];
			actualClass.className = actualClass.className + " last";
		}
	}	
	
/* Listener */
	for(var i=0;i<bInc.length;i++){
        bInc[i].addEventListener('click', function(){increment(this.parentNode.getElementsByTagName('span')[0]);},false);
    }

	for(var j=0;j<bDec.length;j++){
        bDec[j].addEventListener('click', function(){decrement(this.parentNode.getElementsByTagName('span')[0]);}, false);
    }

	bAdd[0].addEventListener('click', addClass, false);	
	
	bRemove[0].addEventListener('click', removeClass, false);
	
	/* Initialisation */
	document.getElementsByClassName('classes')[0].style.display = "none";
	document.getElementsByClassName('classes')[1].style.display = "none";
	document.getElementsByClassName('classes')[2].style.display = "none";
	document.getElementsByClassName('classes')[3].style.display = "table-cell";

	if (window.widget) {
        window.widget.onleave = function(){			
            window.sankore.setPreference("savedNumber", getNumber());			
			if(vLastClass.indexOf(' last')>=0){ 
				vLastClass = vLastClass.replace(' last',''); 
			} 			
            window.sankore.setPreference("lastClass", vLastClass);
        }
    }
	
	if(window.sankore){
		var savedNumber = window.sankore.preference('savedNumber');
		var lastClass = window.sankore.preference('lastClass');
		setNumber(savedNumber, lastClass);		
    }