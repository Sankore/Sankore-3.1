var compteurebul = 0;

/*définition de nouvelles variables (pas pareilles que pour fonction_fusion, car sinon les fonctions ne fonctionnent pas dans uniboard */
var tnegb = "#08e8fc";
var t500b = "#07fda1";
var t1000b = "#0bf944";
var t1500b = "#2efa00";
var t2000b = "#b8fa00";
var t2500b = "#f0fa00";
var t3000b = "#fac300";
var t3500b = "#f88a01";
var t4000b = "#fa5100";
var t4500b = "#fa3000";
var t5000b = "#f50404";
var t5500b = "#de0000";
var t6000b = "#a80000";


function ebul() {

	if (compteurebul == 0) {

		document.getElementById("div_legendes").innerHTML = "<img src='Images/leg_fus_ebul.png'>";
		
		/*éléments qui ont la température de fusion la plus basse, T = -273..0 */
		
		for	(var i=1; i <= 2; i++) {
			document.getElementById(i).style.backgroundColor = tnegb;
		}	
			
		for	(var i=7; i <= 10; i++) {
			document.getElementById(i).style.backgroundColor = tnegb;
		}
		
		for	(var i=17; i <= 18; i++) {
			document.getElementById(i).style.backgroundColor = tnegb;
		}
		
		document.getElementById('36').style.backgroundColor = tnegb;
		document.getElementById('86').style.backgroundColor = tnegb;
		document.getElementById('54').style.backgroundColor = tnegb;
		

		/* éléments qui ont la ..500*/
		
		for	(var i=15; i <= 16; i++) {
			document.getElementById(i).style.backgroundColor = t500b;
		}

		document.getElementById('35').style.backgroundColor = t500b;
		document.getElementById('53').style.backgroundColor = t500b;
		document.getElementById('80').style.backgroundColor = t500b;
		document.getElementById('85').style.backgroundColor = t500b;
		
		
		/* 500..1000 */
		
		document.getElementById('11').style.backgroundColor = t1000b;
		document.getElementById('19').style.backgroundColor = t1000b;
		document.getElementById('37').style.backgroundColor = t1000b;
		document.getElementById('87').style.backgroundColor = t1000b;
		document.getElementById('30').style.backgroundColor = t1000b;
		document.getElementById('52').style.backgroundColor = t1000b;
		document.getElementById('84').style.backgroundColor = t1000b;
		document.getElementById('48').style.backgroundColor = t1000b;

		for	(var i=33; i <= 34; i++) {
				document.getElementById(i).style.backgroundColor = t1000b;
		}

		
		/*1000..1500*/
		
		document.getElementById('3').style.backgroundColor = t1500b;
		document.getElementById('12').style.backgroundColor = t1500b;
		document.getElementById('20').style.backgroundColor = t1500b;
		document.getElementById('38').style.backgroundColor = t1500b;
		document.getElementById('55').style.backgroundColor = t1500b;
		document.getElementById('70').style.backgroundColor = t1500b;
		document.getElementById('81').style.backgroundColor = t1500b;
		document.getElementById('88').style.backgroundColor = t1500b;
		document.getElementById('98').style.backgroundColor = t1500b;
		
		
		/*1500..2000*/
		
		document.getElementById('56').style.backgroundColor = t2000b;
		document.getElementById('25').style.backgroundColor = t2000b;
		document.getElementById('51').style.backgroundColor = t2000b;
		document.getElementById('69').style.backgroundColor = t2000b;

		for	(var i=82; i <= 83; i++) {
				document.getElementById(i).style.backgroundColor = t2000b;
		}
		
		for	(var i=62; i <= 63; i++) {
				document.getElementById(i).style.backgroundColor = t2000b;
		}
		
		
		/*2000..2500*/
		
		for	(var i=13; i <= 14; i++) {
				document.getElementById(i).style.backgroundColor = t2500b;
		}
		
		document.getElementById('31').style.backgroundColor = t2500b;
		document.getElementById('47').style.backgroundColor = t2500b;
		document.getElementById('61').style.backgroundColor = t2500b;

		for	(var i=49; i <= 50; i++) {
				document.getElementById(i).style.backgroundColor = t2500b;
		}
		
		
		/*2500..3000 */
		
		document.getElementById('4').style.backgroundColor = t3000b;
		document.getElementById('21').style.backgroundColor = t3000b;
		document.getElementById('24').style.backgroundColor = t3000b;
		
		for	(var i=26; i <= 29; i++) {
				document.getElementById(i).style.backgroundColor = t3000b;
		}
		
		document.getElementById('32').style.backgroundColor = t3000b;
		document.getElementById('46').style.backgroundColor = t3000b;
		document.getElementById('79').style.backgroundColor = t3000b;
		
		for	(var i=66; i <= 68; i++) {
				document.getElementById(i).style.backgroundColor = t3000b;
		}
		
		document.getElementById('95').style.backgroundColor = t3000b;
		
		
		/*3000..3500 */
		
		document.getElementById('39').style.backgroundColor = t3500b;
		document.getElementById('89').style.backgroundColor = t3500b;
		document.getElementById('58').style.backgroundColor = t3500b;
		document.getElementById('60').style.backgroundColor = t3500b;
		document.getElementById('71').style.backgroundColor = t3500b;
		document.getElementById('94').style.backgroundColor = t3500b;
		document.getElementById('57').style.backgroundColor = t3500b;

		for	(var i=22; i <= 23; i++) {
				document.getElementById(i).style.backgroundColor = t3500b;
		}
		
		for	(var i=64; i <= 65; i++) {
				document.getElementById(i).style.backgroundColor = t3500b;
		}
		

		/*3500..4000*/
		
		for	(var i=44; i <= 45; i++) {
				document.getElementById(i).style.backgroundColor = t4000b;
		}
		
		document.getElementById('59').style.backgroundColor = t4000b;
		document.getElementById('78').style.backgroundColor = t4000b;

		for	(var i=92; i <= 93; i++) {
				document.getElementById(i).style.backgroundColor = t4000b;
		}
		
		
		/*4000..4500 */
		
		document.getElementById('5').style.backgroundColor = t4500b;
		document.getElementById('40').style.backgroundColor = t4500b;
		document.getElementById('77').style.backgroundColor = t4500b;
		document.getElementById('91').style.backgroundColor = t4500b;
		
		
		/*4500..5000*/
		
		document.getElementById('6').style.backgroundColor = t5000b;
		document.getElementById('72').style.backgroundColor = t5000b;
		document.getElementById('90').style.backgroundColor = t5000b;
		
		for	(var i=41; i <= 43; i++) {
				document.getElementById(i).style.backgroundColor = t5000b;
		}
		
		
		/*5000..5500*/
		
		document.getElementById('73').style.backgroundColor = t5500b;
		document.getElementById('76').style.backgroundColor = t5500b;
		
		
		/*5500..6000*/
		
		for	(var i=74; i <= 75; i++) {
				document.getElementById(i).style.backgroundColor = t6000b;
		}
		
		/*gris*/
		
		for	(var i=96; i <= 97; i++) {
				document.getElementById(i).style.backgroundColor = "lightgrey";
		}
		
		for	(var i=98; i <= 118; i++) {
				document.getElementById(i).style.backgroundColor = "lightgrey";
		}
		
		document.getElementById("boutonebul").style.backgroundColor = "#7c9cd8";
		document.getElementById("boutonetat").style.backgroundColor = "";
		document.getElementById("boutonelectroneg").style.backgroundColor = "";
		document.getElementById("boutonfusion").style.backgroundColor = "";
		
		compteurebul = 1;
		compteuretat = 0;
		compteurEN = 0;
		compteurfusion = 0;
		

	}
	
	else {
		location.reload();
		compteurebul= 0;
	}
		
	
}
