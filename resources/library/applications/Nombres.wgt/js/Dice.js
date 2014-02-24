var nbDes = 1;
var total = 0;
function changeValeur(nb) 
{	
	
		var retour = document.createElement("input");
		retour.setAttribute('value', 'Retour');
		retour.type = "button";
		retour.setAttribute('onClick', 'creerPage();');
		var menu = document.getElementById("menu");
		menu.innerHTML = ""; 	
		menu.appendChild(retour);
		var contenu = document.getElementById("content");
		contenu.innerHTML = ""; 			
		var valeur = document.getElementById("valeur");
		nbDes = nb;		
		lancerDes();		
		
		
}
function roll(){
	var wait=setTimeout("change();",500);
}
function change(){
total=0;
	for(var i=1; i<=nbDes;i++){
			var nnAlea = Math.floor(Math.random()*10);
			total= total+nnAlea;
			var des = document.getElementById("de"+i);
			des.setAttribute('src', 'images/'+nnAlea+'.png');	
		}	
		var somme = document.createElement("p");
		somme.innerHTML=total;
	
}


function lancerDes(){
		var valeur = document.getElementById("valeur");
		var espace = document.createElement("br");
		valeur.innerHTML="";
		valeur.appendChild(espace);
		for(var i=1; i<=nbDes;i++){
		
			var nnAlea = Math.floor(Math.random()*6)+1;
			var des = document.createElement("img");
			des.setAttribute('src', 'images/tornade.gif');	
			des.setAttribute('HEIGHT', '135');	des.setAttribute('WIDTH', '135');	
			
			des.setAttribute('onclick', 'lancerDes();');
			des.setAttribute('id', 'de'+i);	
			valeur.appendChild(des);     
			
		}	
		var somme = document.createElement("p");
		somme.setAttribute('id', 'somme');	
		
		
		valeur.appendChild(somme);
roll();		
}
function creerPage(){
	var appli = document.getElementById("appli");
	appli.innerHTML="";
	var menu =document.createElement("p");
	menu.id="menu";
	appli.appendChild(menu);
	menu.innerHTML='Choisissez un nombre de d&eacute;s &agrave; lancer';
	var content =document.createElement("p");
	content.id="content";
	for (var i=1;i<=5;i++){
		var lien =  document.createElement("a");
		var bouton = document.createElement("img");
		bouton.setAttribute('src', 'images/'+i+'.png');
		lien.setAttribute('onClick', 'changeValeur('+i+');');
		lien.appendChild(bouton);
		content.appendChild(lien);
	}
	appli.appendChild(content);
	var valeur =document.createElement("div");
	valeur.id="valeur";
	appli.appendChild(valeur);
	
}
