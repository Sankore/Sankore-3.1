compteurEN = 0;

function electroneg() {
	
	if (compteurEN==0) {
	
		document.getElementById("div_legendes").innerHTML = "<img src='Images/leg_electroneg.png'>";
		
		/*éléments les moins électronégatifs, EN = 0-0.9 */
		document.getElementById('11').style.backgroundColor = "yellow";
		for	(var i=19; i <= 55; i=i+18) {
			document.getElementById(i).style.backgroundColor = "yellow";
		}
		
		for	(var i=56; i <= 88; i=i+32) {
			document.getElementById(i).style.backgroundColor = "yellow";
		}
		
		document.getElementById('87').style.backgroundColor = "yellow";
		
		/*éléments EN = 1-1.9 */
		document.getElementById('3').style.backgroundColor = "#FFB900";
		document.getElementById('4').style.backgroundColor = "#FFB900";
		document.getElementById('12').style.backgroundColor = "#FFB900";
		for	(var i=12; i <= 13; i++) {
			document.getElementById(i).style.backgroundColor = "#FFB900";
		}
		
		for	(var i=20; i <= 30; i++) {
			document.getElementById(i).style.backgroundColor = "#FFB900";
		}
		
		for	(var i=38; i <= 43; i++) {
			document.getElementById(i).style.backgroundColor = "#FFB900";
		}
		
		for	(var i=47; i <= 49; i++) {
			document.getElementById(i).style.backgroundColor = "#FFB900";
		}
		
		for	(var i=57; i <= 75; i++) {
			document.getElementById(i).style.backgroundColor = "#FFB900";
		}
		document.getElementById('80').style.backgroundColor = "#FFB900";
		
		for	(var i=89; i <= 103; i++) {
			document.getElementById(i).style.backgroundColor = "#FFB900";
		}
		
		/*éléments EN = 2-2.9 */
		document.getElementById('1').style.backgroundColor = "#FF7200";
		
		for	(var i=5; i <= 7; i++) {
			document.getElementById(i).style.backgroundColor = "#FF7200";
		}
		
		for	(var i=14; i <= 16; i++) {
			document.getElementById(i).style.backgroundColor = "#FF7200";
		}
		
		for	(var i=31; i <= 34; i++) {
			document.getElementById(i).style.backgroundColor = "#FF7200";
		}
		
		for	(var i=44; i <= 46; i++) {
			document.getElementById(i).style.backgroundColor = "#FF7200";
		}
		
		for	(var i=50; i <=53; i++) {
			document.getElementById(i).style.backgroundColor = "#FF7200";
		}
		
		for	(var i=76; i <=79; i++) {
			document.getElementById(i).style.backgroundColor = "#FF7200";
		}
		
		for	(var i=81; i <=85; i++) {
			document.getElementById(i).style.backgroundColor = "#FF7200";
		}
		
		/*éléments EN = 3-3.9 */
		document.getElementById('8').style.backgroundColor = "#FF2A00";
		document.getElementById('9').style.backgroundColor = "#EF0000";
		document.getElementById('17').style.backgroundColor = "#FF2A00";
		document.getElementById('35').style.backgroundColor = "#FF2A00";
		
		/*éléments gris*/
		for	(var i=2; i <=18; i=i+8) {
			document.getElementById(i).style.backgroundColor = "lightgrey";
		}
		
		document.getElementById('36').style.backgroundColor = "lightgrey";
		document.getElementById('54').style.backgroundColor = "lightgrey";
		document.getElementById('86').style.backgroundColor = "lightgrey";
		
		for	(var i=104; i <=118; i++) {
			document.getElementById(i).style.backgroundColor = "lightgrey";
		}
		
		compteuretat = 0;
		compteurEN = 1;
		compteurfusion = 0;
		compteurebul = 0;
		
		document.getElementById("boutonelectroneg").style.backgroundColor = "#7c9cd8";
		document.getElementById("boutonetat").style.backgroundColor = "";
		document.getElementById("boutonebul").style.backgroundColor = "";
		document.getElementById("boutonfusion").style.backgroundColor = "";
	}
	
	else {
		location.reload();
		compteurEN = 0;
	}
	
}