
function infos(element, numa, masseatomique, electroneg, ptfus, ptebul) {
	
	var a = document.getElementById("checklewis").checked; /* variable a = true si la case lewis est cochée, sinon a = false */
	
	/* si le nom de la case Lewis est cochée, la fonction affiche le nom de l'élément et sa représentation de lewis dans la 'div_infos' */
	if (a == true) {
		document.getElementById("div_infos").innerHTML = "<h3>"+element+"</h3>" +'<img src="Images/'+numa+'.svg">';	
	}
	
	/* si la case n'est pas cochée, la fonction affiche les infos relatives à l'élément dans la 'div_infos' */
	else {
		document.getElementById("div_infos").innerHTML = "<h3>"+element+"</h3>" + "<table id='table_infos'>"
																						+"<tr>"
																							+"<td class='colonne1'>Numéro atomique:</td>"
																							+"<td class='colonne2'>" +numa+ "</td>"
																							+"<td class='colonne3'>Protons:</td>"
																							+"<td>"+numa+"</td>"
																						+"</tr>"
																						+"<tr>"
																							+"<td class='colonne1'>Masse atomique:</td>"
																							+"<td class='colonne2'>" +masseatomique+ "</td>"
																							+"<td class='colonne3'>Electrons:</td>"
																							+"<td>"+numa+"</td>"
																						+"</tr>"
																						+"<tr>"
																							+"<td class='colonne1'>Electronégativité:</td>"
																							+"<td class='colonne2'>"+electroneg+"</td>"
																							+"<td class='colonne3'>Neutrons:</td>"
																							+"<td>"+parseInt(masseatomique-numa)+"</td>"
																						+"</tr>"
																						+"<tr>"
																							+"<td class='colonne1'>Point de fusion:</td>"
																							+"<td class='colonne2'>"+ptfus+"</td>"
																						+"</tr>"
																						+"<tr>"
																							+"<td class='colonne1'>Point d'ébullition:</td>"
																							+"<td class='colonne2'>"+ptebul+"</td>"
																						+"</tr>"
																					+"</table>";
	}	
		
}
