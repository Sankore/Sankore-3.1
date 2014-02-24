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

Date.prototype.addDays = function(days){
    this.setDate(this.getDate() + days);
};

function refreshDate(vue, date)
{
	vue.jourSemaine.textContent = jours[date.getDay()];
	vue.jourAnnee.textContent = date.getDate();
	vue.mois.textContent = mois[date.getMonth()];
	vue.annee.textContent = date.getFullYear();
}

/*---------------------------------*/
//Compote et liqueure de frelons ne font du bien qu'à cupidon

var vue = {

	jourSemaine: document.querySelector("#jourSemaine"),
	jourAnnee: document.querySelector("#jourAnnee"),
	mois: document.querySelector("#mois"),
	annee: document.querySelector("#annee"),
};

var date = new Date();

var buttons = {

	previous: document.querySelector("#previous"),
	next: document.querySelector("#next")

};

previous.onclick = function(){

	date.addDays(-1);
	refreshDate(vue, date);

};

next.onclick = function(){

	date.addDays(1);
	refreshDate(vue, date);

};

refreshDate(vue, date);