//Variables de référence

var jours = [
	"Dimanche",
	"Lundi",
	"Mardi",
	"Mercredi",
	"Jeudi",
	"Vendredi",
	"Samedi"
];

var mois = [
	"Janvier",
	"Février",
	"Mars",
	"Avril",
	"Mai",
	"Juin",
	"Juillet",
	"Aout",
	"Septembre",
	"Octobre",
	"Novembre",
	"Décembre"
];

/*---------------------------------*/
//Fonctions de référence

function pickMorale(){

	var index = Math.floor(Math.random() * quotes.length) ;
	
	return {
		"quote": quotes[index][0],
		"author": quotes[index][1]
	};
}

function showDate(){

	var date = new Date();
	return jours[date.getDay()] + " " + date.getDate() + " " + mois[date.getMonth()] + ' ' + date.getFullYear() ;
}

function refreshDate(vue, date)
{
	vue.date.textContent = showDate();

	var morale = pickMorale();

	vue.quote.textContent = morale.quote ;
	vue.author.textContent = morale.author ;

}

/*---------------------------------*/
//Compote et liqueur de frelons ne font du bien qu'à cupidon

var vue = {

	date: document.querySelector("#date"),
	quote: document.querySelector("#morale"),
	author: document.querySelector("#author")
};

document.querySelector("a").onclick = function()
{
	refreshDate(vue, date);
};

refreshDate(vue, date);