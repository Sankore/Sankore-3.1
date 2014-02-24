	var solide = "#b5b5b5";
	var liquide = "#21cbce";
	var gaz = "#ca43d1";
	var synth = "#34eb29";
	var compteuretat = 0; /* compteur local pour que lorsqu'on clique sur le bouton,
		les couleurs s'affichent selon les états, et si l'on reclique
		sur ce bouton, les couleurs de base se raffichent. Le compteur est remis à
		zéro si l'on clique sur un autre bouton (qui actionne une autre
		fonction). Ainsi on peut cliquer sur d'autres boutons et recliquer sur celui
		des états sans que les couleurs de base se raffichent. */
	
function etat()	{
	
	if (compteuretat==0) {
	
		/*légendes: (sous forme de tableau)*/
		document.getElementById("div_legendes").innerHTML = "<table><tr><td id='solide'>Solide</td><td id='liquide'>Liquide</td><td id='gaz'>Gaz</td><td id='synth'>Synthétique</td></table>";

		for	(var i=3; i <= 6; i++) {
			document.getElementById(i).style.backgroundColor = solide;
		}
		
		for	(var i=11; i <= 16; i++) {
			document.getElementById(i).style.backgroundColor = solide;
		}

		for	(var i=19; i <= 34; i++) {
			document.getElementById(i).style.backgroundColor = solide;
		}
		
		for	(var i=37; i <= 42; i++) {
			document.getElementById(i).style.backgroundColor = solide;
		}
		
		for	(var i=44; i <= 53; i++) {
			document.getElementById(i).style.backgroundColor = solide;
		}
		
		
		for	(var i=55; i <= 60; i++) {
			document.getElementById(i).style.backgroundColor = solide;
		}
		
		for	(var i=62; i <= 79; i++) {
			document.getElementById(i).style.backgroundColor = solide;
		}
		
		for	(var i=81; i <= 85; i++) {
			document.getElementById(i).style.backgroundColor = solide;
		}
		
		for	(var i=87; i <= 92; i++) {
			document.getElementById(i).style.backgroundColor = solide;
		}
		
		for	(var i=93; i <= 118; i++) {
			document.getElementById(i).style.backgroundColor = synth;
		}
		
		document.getElementById("43").style.backgroundColor = synth;
		document.getElementById("61").style.backgroundColor = synth;

		for	(var i=1; i <= 2; i++) {
			document.getElementById(i).style.backgroundColor = gaz;
		}
		
		for	(var i=7; i <= 10; i++) {
			document.getElementById(i).style.backgroundColor = gaz;
		}
		
		for	(var i=17; i <= 18; i++) {
			document.getElementById(i).style.backgroundColor = gaz;
		}
		
		document.getElementById("36").style.backgroundColor = gaz;
		document.getElementById("54").style.backgroundColor = gaz;
		document.getElementById("86").style.backgroundColor = gaz;
		
		document.getElementById("35").style.backgroundColor = liquide;
		document.getElementById("80").style.backgroundColor = liquide;
		
		
		document.getElementById("boutonetat").style.backgroundColor = "#7c9cd8";
		document.getElementById("boutonelectroneg").style.backgroundColor = "";
		document.getElementById("boutonebul").style.backgroundColor = "";
		document.getElementById("boutonfusion").style.backgroundColor = "";

		compteuretat = 1;
		compteurEN = 0;
		compteurfusion = 0;
		compteurebul= 0;
	}
	
	else {
		location.reload();
		compteuretat = 0;
	}		
	
}