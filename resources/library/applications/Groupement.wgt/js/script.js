/* 
Author: Frédéric Misery
Version : 20/12/2012
*/
$(document).ready(function() {
  var representations={
    picbille:{millier:'caisse',centaine:'valise',dizaine:'boite',unite:'bille'},
    cubus:{millier:'bloc',centaine:'plaque',dizaine:'barre',unite:'cube'},
    tickets:{millier:'enveloppeA4',centaine:'enveloppeA6',dizaine:'carnet',unite:'ticket'},
    mots:{millier:'sMillier',centaine:'sCentaine',dizaine:'sDizaine',unite:'sUnite'},
    //      Ligne à ajouter pour ajouter une représentation
    nombres:{millier:'nMillier',centaine:'nCentaine',dizaine:'nDizaine',unite:'nUnite'}
  };
  var representation;
  $('#representation option[value=cubus]').attr('selected','selected');

  // Changer de représentation
  $('#representation').change(function() {
    representation=$('#representation option:selected').attr('value');
    var nbreImages=$('img').length;
    var nbreImagesChargees=0;
    $('img').each(function(){
      $(this).attr('src','images/'+representations[representation][$(this).attr('alt')]+'.png');
      this.onload=function() {
        nbreImagesChargees++;
        if (nbreImagesChargees==nbreImages) {
          $('#tableau').height(hauteurTableau());
        }
      };
    });
  }).change(); 

  var nbreImages=0;
  $('img').each(function(){
    this.onload=function(){
      nbreImages++;
      if (nbreImages==4) $('#tableau').height(hauteurTableau());
    };
  });

  // Optimisation de la hauteur du tableau par rapport à la fenetre du navigateur
  var hauteurTableau=function() {
    var hWindow=$(window).innerHeight();
    var h=hWindow;
    h-=parseInt($('table #commandes').outerHeight());
    h-=parseInt($('table #plein').outerHeight());
    h-=parseInt($('#effacer').innerHeight());
    return h;
  };

  $('#tableau').height(hauteurTableau());

  // Les images de base peuvent être déposées dans le tableau
  $('#plein img').draggable({
    helper:"clone",
    containment:"table"
  });

  // Configuration du comportement lorsque le tableau reçoit une image
  $('#tableau').droppable({
    drop:function(event,ui) {
      if (!$(ui.draggable).hasClass('affiche')) {
        var nouveau=ui.draggable.clone();
        var x = ui.helper.clone().css('left');
        var y = ui.helper.clone().css('top');
        var yMin=$('header').outerHeight()+$('#commandes').outerHeight()+$('#plein').outerHeight()+10;
        y=Math.max(parseInt(y.replace('px','')),yMin)+'px';

        $('#tableau').append(nouveau);
        $(nouveau)
          .css({position:"absolute",top:y,left:x})
          .addClass('affiche');
        if ($('#addition').hasClass('addition_on')) $(nouveau).addClass('addition');
        animerGroupe(nouveau);
        affichageNumerique();
      }
    }
  });

  var nombre2string=function(n) {
    return (n>999) ? n.toString().replace(/(\d*)(\d{3})/g,"$1 $2") : n.toString();
  };
  
  var affichageNumerique=function() {
    var tot=0,plus=0,moins=0;
    $('#total').empty();
    $('img.affiche').each(
      function() {
        switch ($(this).attr('alt')) {
          case 'millier' : tot+=1000; break;
          case 'centaine' : tot+=100; break;
          case 'dizaine' : tot+=10; break;
          case 'unite' : tot+=1; break;
        }
      }
    );
    $('img.addition').each(
      function() {
        switch ($(this).attr('alt')) {
          case 'millier' : plus+=1000; break;
          case 'centaine' : plus+=100; break;
          case 'dizaine' : plus+=10; break;
          case 'unite' : plus+=1; break;
        }
      }
    );
    $('img.soustraction').each(
      function() {
        switch ($(this).attr('alt')) {
          case 'millier' : moins+=1000; break;
          case 'centaine' : moins+=100; break;
          case 'dizaine' : moins+=10; break;
          case 'unite' : moins+=1; break;
        }
      }
    );
    if (plus==0 && moins==0) $('#total').text(nombre2string(tot));
    if (plus!=0) $('#total').text(nombre2string(tot-plus)+' + '+nombre2string(plus));
    if (moins!=0) $('#total').text(nombre2string(tot)+' - '+nombre2string(moins));
  };

  var animerGroupe=function(groupe){
    $(groupe).draggable({
      helper:"original",
      containment:"#tableau"
    });
    $(groupe).click(function(){
      actionClique(this);
    });
  };

  var actionClique=function(groupe) {
    if ($('#addition').hasClass('addition_on')) return true;
    if ($('#soustraction').hasClass('soustraction_on')) selection4soustraction(groupe);
    if ($('#groupement').hasClass('groupement_on')) selection4groupement(groupe);
    if ($('#casse').hasClass('casse_on')) casser(groupe);
    affichageNumerique();
  };

  var liberer=function(action) {
    if ($('.'+action).length!=0 && !confirm('Tu souhaites abandonner ton action ('+action+') en cours ?')) return false; 
    if (action=='addition') $('img.addition').remove();
    $('img').removeClass(action);
    $('#'+action)
      .removeClass(action+'_on')
      .addClass(action+'_off');
    affichageNumerique();
    return true;
  };
  
  var activer=function(action) {
    if (!liberer('addition')) return false;
    if (!liberer('soustraction')) return false;
    if (!liberer('groupement')) return false;
    if (!liberer('casse')) return false;
    $('#'+action)
      .removeClass(action+'_off')
      .addClass(action+'_on');
    affichageNumerique();
  };

  $('#capturer').click(function() {
    if ($('#tableau img').length==0) return true;
    var delta=parseInt($('table #commandes').outerHeight());
    delta+=parseInt($('table #plein').outerHeight());
    
    var canvas=document.createElement('canvas');
    var ctx=canvas.getContext('2d');
    canvas.width=$('#tableau').width();
    canvas.height=$('#tableau').height();
    ctx.fillStyle='white';
    ctx.fillRect(0,0,canvas.width,canvas.height);
    var nbreImagesChargees=0;
    var ajouterImage=function(src,x,y) {
      var img=new Image();
      img.src=src;
      img.onload=function() {
        nbreImagesChargees++;
        ctx.drawImage(this,x,y);
        if (nbreImagesChargees==$('#tableau img').length) sankore.addObject(canvas.toDataURL("image/png"));
      };
    };

    $('#tableau img').each(function(i) {
      ajouterImage(this.src,parseInt($(this).css('left')),parseInt($(this).css('top'))-delta);
    });
  });

  $('#effacer').click(function(){
    if ($('#tableau img').length==0) return true;
    if (confirm('Tous les objets déposés sur le tableau vont-être effacés.')) {
      $('#tableau img').remove();
      liberer('addition');
      liberer('soustraction');
      liberer('casse');
      liberer('groupement');
    }
  });

  $('#addition').click(function() {
    if ($(this).hasClass('addition_on')) {
      $('img').removeClass('addition');
      liberer('addition');
    } else {
      activer('addition');
    }
  });

  $('#soustraction').click(function() {
    if ($(this).hasClass('soustraction_on')) {
      soustraire();
      liberer('soustraction');
    } else {
      activer('soustraction');
    }
  });

  $('#groupement').click(function() {
    if ($(this).hasClass('groupement_on')) {
      if (grouper()) liberer('groupement');
    } else {
      activer('groupement');
    }
  });

  $('#casse').click(function() {
    ($(this).hasClass('casse_on')) ?
      liberer('casse'):
      activer('casse');
  });

  $('#afficher').click(function() {
    if ($(this).hasClass('affichage_on')) {
      $(this)
        .removeClass('affichage_on')
        .addClass('affichage_off');
      $('#total').removeClass('voir');
    } else {
      $(this)
        .removeClass('affichage_off')
        .addClass('affichage_on');
      $('#total').addClass('voir');
    }
  });

  var selection4soustraction=function(objet) {
    if ($(objet).hasClass('soustraction')) {        //     Le groupe était sélectionné on le déselectionne
      $(objet).removeClass('soustraction');
    } else {                                      //     Sinon on sélectionne le groupe
      $(objet).addClass('soustraction');
    }
  };

  var selection4groupement=function(objet) {
    if ($(objet).hasClass('groupement')) {        //     Le groupe était sélectionné on le déselectionne
      $(objet).removeClass('groupement');
    } else {                                     //     On selectionne le groupe
      var groupement=$(objet).attr('alt');
      if (groupement=='millier') {             //     sauf les milliers
        alert('Le groupement de milliers (dizaine de milliers) n\'est pas géré par l\'application.');
      } else {
        if ($('.groupement').length!=0 && $('.groupement:eq(0)').attr('alt')==groupement) {  //    Même groupe
          $(objet).addClass('groupement');
        } else {                     // Groupe différent, on ne peut pas grouper
          $('img').removeClass('groupement'); // Déselection ancien groupe
          $(objet).addClass('groupement');     // Sélection nouveau groupe
        }
      }
    }
  };

  var soustraire=function() {
    $('.soustraction').remove();
  };

  var grouper=function() {
    if ($('img.groupement').length!=0 && $('img.groupement').length!=10) {
      alert('Le nombre d\'objets sélectionnés ne correspond pas à un groupement !');
      return false;
    } else {
      var groupements=$('.groupement:eq(0)');
      switch (groupements.attr('alt')) {
        case 'centaine':
          groupements
            .attr({src:'images/'+representations[representation].millier+'.png',alt:'millier',title:'Un millier'})
            .removeClass('groupement');
          break;
        case 'dizaine':
          groupements
            .attr({src:'images/'+representations[representation].centaine+'.png',alt:'centaine',title:'Une centaine'})
            .removeClass('groupement');
          break;
        case 'unite':
          groupements
            .attr({src:'images/'+representations[representation].dizaine+'.png',alt:'dizaine',title:'Une dizaine'})
            .removeClass('groupement');
            break;
        default :
          break;
      }
      $('.groupement').remove();
      return true;
    }
  };

  var casser=function(objet) {
    var groupement=$(objet).attr('alt');
    if (groupement=='unite') {
      alert('La casse des unités (dixièmes) n\'est pas gérée par l\'application.');
      return false;
    }
    if (!confirm('Tu souhaites casser ce groupe ?')) return false;
    switch(groupement) {
      case 'millier':
        $(objet).attr({src:'images/'+representations[representation].centaine+'.png',alt:'centaine',title:'Une centaine'});
        break;
      case 'centaine':
        $(objet).attr({src:'images/'+representations[representation].dizaine+'.png',alt:'dizaine',title:'Une dizaine'});
        break;
      case 'dizaine':
        $(objet).attr({src:'images/'+representations[representation].unite+'.png',alt:'unite',title:'Une unite'});
        break;
      default :
        return false; 
        break;
    }
    for (var i=0;i<10;i++) {
      var copie=$(objet).clone();
      $(objet).after(copie);
    }
    degrouper(objet);
    $('#casse').click();
  };

  var degrouper=function(objet) {
    var wTableau=$('#tableau').innerWidth();
    var hTableau=$('#tableau').innerHeight();
    var x0=Math.max(parseInt($(objet).css('left').replace('px','')),80);
    var y0=Math.min(parseInt($(objet).css('top').replace('px','')),hTableau-(40+$(objet).outerHeight()));
    $(objet).nextAll('img:lt(10)').each(function(i){
      var x=(x0-7*i)+'px';
      var y=(y0+20*i)+'px';
      $(this).css({top:y,left:x});
      animerGroupe(this);
    });
    $(objet).remove();
  };
});
