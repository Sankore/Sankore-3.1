var tneg = "#08e8fc";
var t250 = "#07fda1";
var t500 = "#0bf944";
var t750 = "#2efa00";
var t1000 = "#b8fa00";
var t1250 = "#f0fa00";
var t1500 = "#fac300";
var t2000 = "#f88a01";
var t2500 = "#fa5100";
var t3000 = "#fa3000";
var t3500 = "#f50404";
var t4000 = "#de0000";


var compteurfusion = 0;

function fusion() {

	if (compteurfusion == 0) {
	
		document.getElementById("div_legendes").innerHTML = "<img src='Images/leg_fus_ebul.png'>";
		
		
		/*éléments qui ont la température de fusion la plus basse, T = -273..0 */
		for	(var i=1; i <= 2; i++) {
			document.getElementById(i).style.backgroundColor = tneg;
		}	
		
		for	(var i=7; i <= 10; i++) {
			document.getElementById(i).style.backgroundColor = tneg;
		}	
		
		for	(var i=17; i <= 35; i=i+18) {
			document.getElementById(i).style.backgroundColor = tneg;
		}	
		
		for	(var i=18; i <= 54; i=i+18) {
			document.getElementById(i).style.backgroundColor = tneg;
		}
		
		document.getElementById('80').style.backgroundColor = tneg;
		document.getElementById('86').style.backgroundColor = tneg;
		
		
		/*éléments qui ont une température de fusion moyennement basse, T = 0..250*/
		for	(var i=3; i <= 11; i=i+8) {
			document.getElementById(i).style.backgroundColor = t250;
		}	
		
		for	(var i=19; i <= 37; i=i+18) {
			document.getElementById(i).style.backgroundColor = t250;
		}	
		
		for	(var i=55; i <= 87; i=i+32) {
			document.getElementById(i).style.backgroundColor = t250;
		}	
		
		for	(var i=15; i <= 16; i++) {
			document.getElementById(i).style.backgroundColor = t250;
		}	
		
		document.getElementById('31').style.backgroundColor = t250;
		document.getElementById('34').style.backgroundColor = t250;

		for	(var i=49; i <= 50; i++) {
			document.getElementById(i).style.backgroundColor = t250;
		}
		
		for	(var i=15; i <= 16; i++) {
			document.getElementById(i).style.backgroundColor = t250;
		}
		
		document.getElementById('53').style.backgroundColor = t250;
		
		
		/*éléments qui ont une temp. de fusion moyenne, T = 250..500*/
		
		document.getElementById('30').style.backgroundColor = t500;
		document.getElementById('48').style.backgroundColor = t500;
		document.getElementById('52').style.backgroundColor = t500;

		for	(var i=81; i <= 85; i++) {
			document.getElementById(i).style.backgroundColor = t500;
		}	
		
		
		/*éléments qui ont une temp. de fusion plutôt élevée, T = 500..750*/
		
		document.getElementById('12').style.backgroundColor = t750;
		document.getElementById('13').style.backgroundColor = t750;
		document.getElementById('56').style.backgroundColor = t750;
		document.getElementById('88').style.backgroundColor = t750;
		document.getElementById('51').style.backgroundColor = t750;

		for	(var i=93; i <= 94; i++) {
			document.getElementById(i).style.backgroundColor = t750;
		}
		
		
		/* T = 750..1000 */
		
		document.getElementById('20').style.backgroundColor = t1000;
		document.getElementById('38').style.backgroundColor = t1000;
		document.getElementById('32').style.backgroundColor = t1000;
		document.getElementById('47').style.backgroundColor = t1000;
		
		document.getElementById('55').style.backgroundColor = t1000;
		
		for	(var i=57; i <= 59; i++) {
			document.getElementById(i).style.backgroundColor = t1000;
		}
		
		document.getElementById('63').style.backgroundColor = t1000;
		document.getElementById('70').style.backgroundColor = t1000;
		document.getElementById('95').style.backgroundColor = t1000;
		
		for	(var i=101; i <= 102; i++) {
			document.getElementById(i).style.backgroundColor = t1000;
		}
		
		
		/* T = 1000..1250 */
		for	(var i=25; i <= 29; i=i+4) {
			document.getElementById(i).style.backgroundColor = t1250;
		}
		
		for	(var i=79; i <= 89; i=i+10) {
			document.getElementById(i).style.backgroundColor = t1250;
		}
		
		for	(var i=60; i <= 62; i++) {
			document.getElementById(i).style.backgroundColor = t1250;
		}
		
		document.getElementById('92').style.backgroundColor = t1250;
		
		
		/* T = 1250..1500*/
		
		for	(var i=4; i <= 14; i=i+10) {
			document.getElementById(i).style.backgroundColor = t1500;
		}
		
		
		for	(var i=27; i <= 28; i++) {
			document.getElementById(i).style.backgroundColor = t1500;
		}
		
		for	(var i=64; i <= 67; i++) {
			document.getElementById(i).style.backgroundColor = t1500;
		}

		document.getElementById('96').style.backgroundColor = t1500;

		
		/* T = 1500..2000*/
		
		for	(var i=21; i <= 24; i++) {
			document.getElementById(i).style.backgroundColor = t2000;
		}
		
		document.getElementById('26').style.backgroundColor = t2000;
		document.getElementById('39').style.backgroundColor = t2000;
		document.getElementById('40').style.backgroundColor = t2000;

		for	(var i=45; i <= 46; i++) {
			document.getElementById(i).style.backgroundColor = t2000;
		}

		document.getElementById('78').style.backgroundColor = t2000;
		document.getElementById('71').style.backgroundColor = t2000;
		document.getElementById('103').style.backgroundColor = t2000;
		
		for	(var i=68; i <= 69; i++) {
			document.getElementById(i).style.backgroundColor = t2000;
		}
		
		for	(var i=90; i <= 91; i++) {
			document.getElementById(i).style.backgroundColor = t2000;
		}

		
		/* T = 2000..2500 */
		
		document.getElementById('5').style.backgroundColor = t2500;
		document.getElementById('41').style.backgroundColor = t2500;
		document.getElementById('72').style.backgroundColor = t2500;
		document.getElementById('77').style.backgroundColor = t2500;

		for	(var i=43; i <= 44; i++) {
			document.getElementById(i).style.backgroundColor = t2500;
		}
		
		
		/* T = 2500..3000 */

		document.getElementById('42').style.backgroundColor = t3000;
		document.getElementById('73').style.backgroundColor = t3000;

		
		/* T = 3000..3500 */
		
		for	(var i=74; i <= 76; i++) {
			document.getElementById(i).style.backgroundColor = t3500;
		}

		
		/* T = 3500..4000 */
		
		document.getElementById('6').style.backgroundColor = t4000;
		
		
		/*gris*/
		
		for	(var i=97; i <= 100; i++) {
			document.getElementById(i).style.backgroundColor = "lightgrey";
		}
		
		for	(var i=104; i <= 118; i++) {
			document.getElementById(i).style.backgroundColor = "lightgrey";
		}
		
		document.getElementById('33').style.backgroundColor = "lightgrey";
		
		
		
		document.getElementById("boutonfusion").style.backgroundColor = "#7c9cd8";
		document.getElementById("boutonetat").style.backgroundColor = "";
		document.getElementById("boutonelectroneg").style.backgroundColor = "";
		document.getElementById("boutonebul").style.backgroundColor = "";
		
		compteurfusion = 1;
		compteuretat = 0;
		compteurEN = 0;
		compteurebul = 0;
		
	}
	
	else {
		location.reload();
		compteurfusion = 0;
	}
}
	