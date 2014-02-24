var compteur = 0;
var taille = 0;

function positions() {

	compteur = compteur + 1;
	
	if (compteur%2==0) {
		document.getElementById("div_tab2").style.top = "430px";
		document.getElementById("div_tab2").style.left = "262px";
		document.getElementById("div_tab1b").style.left = "262px";
		document.getElementById("minitableau").src = "Images/minitableaulong.png";
		taille=0;
	}
	else {
		document.getElementById("div_tab2").style.top = "325px";
		document.getElementById("div_tab2").style.left = "259px";
		document.getElementById("div_tab1b").style.left = "791px";
		document.getElementById("minitableau").src = "Images/minitableaubloc.png";
		taille=1;
	} 
}

function agrandirdiv() {
	while (taille<=1) {
		document.getElementById("div_infos").style.width= longueur+2+ "px";
		document.getElementById("div_infos").style.height= largeur+1 +"px";
		taille= taille+0.1;
	}
}
		
	
/*
var colon=1;
while (colon<=3) {
	
		var colonne = "col"+1;
		document.getElementById(colonne).style.top= "200px";
		document.getElementById(colonne).style.left = 170+colon*25 + "px";
		document.getElementById("div_tab1b").style.left= 300+colon*25 + "px";
		colon=colon+1;
	}
}
*/

