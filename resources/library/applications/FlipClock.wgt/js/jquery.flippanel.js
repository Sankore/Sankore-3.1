/* Jeff Keen
   http://keen.me
*/

!function ($) {

  "use strict"; // jshint ;_;

  var FlipPanel = function (element, options) {
    this.init('flippanel', element, options)
  }

  FlipPanel.prototype = {
    constructor: FlipPanel,
    init: function (type, element, options) {
      var eventIn
        , eventOut
      this.type = type
      this.$element = $(element)
      this.options = this.getOptions(options)
      this.enabled = true
      this.$element.addClass('flip-panel');
      this.contents = this.$element.html();
      this.$element.html(this.options.template);
      this.setCurrent(this.contents);
    },
    setCurrent: function(current) {
      this.current = current;
      this.$element.find('[data-current]').each(function() {
        $(this).html(current);
      })
    },
    setNext: function(next) {
      this.next = next;
      this.$element.find('[data-next]').each(function() {
        $(this).html(next);
      })      
    },
    change: function(newValue, speed) {
      if (!speed) {
        speed = this.options.speed
      }
      if (this.current == newValue) { return; }
      
      this.setNext(newValue);
      var txtime = speed / 1000;
      
      this.$element.find('.layer.top')
        .css('-moz-transition', 'all ' + txtime + 's ease-in-out')
        .css('-webkit-transition', 'all ' + txtime + 's ease-in-out')
        .css('transition', 'all ' + txtime + 's ease-in-out');
        
      this.$element.find('.layer.top')
        .css('-webkit-transform', 'rotateX(-180deg)')
        .css('-moz-transform', 'rotateX(-180deg)')
        .css('transform', 'rotateX(-180deg)');

      this.reload(newValue, speed)
    },
    reload: function(value, speed) {
      var _this = this;
      window.setTimeout(function() {
        // Reset values half way through transition so we don't get backface visibility bleedthrough
        _this.setCurrent(value);
      }, (speed * 0.75))
      
      window.setTimeout(function() {
        _this.$element.find('.layer.top')
          .css('-moz-transition', 'none')
          .css('-webkit-transition', 'none')
          .css('transition', 'none');
        
        _this.$element.find('.layer.top')
          .css('-webkit-transform', 'rotateX(0deg)')
          .css('-moz-transform', 'rotateX(0deg)')
          .css('transform', 'rotateX(0deg)');
        
      }, speed)
    },
    run: function(values) {
      var timeIncrement = (1000/(values.length));
      var that = this;
      var index = 0;
      var delayChange = function() {
        window.setTimeout(function() {
          that.change(values[index], timeIncrement);
          if (index < values.length) {
            index = index + 1;
            delayChange();
          }
        }, timeIncrement)
      }
      delayChange();
    },
    getOptions: function (options) {
      options = $.extend({}, $.fn[this.type].defaults, options, this.$element.data())

      return options
    }
  }

  $.fn.flippanel = function ( option ) {
    return this.each(function () {
      var $this = $(this)
        , data = $this.data('flippanel')
        , options = typeof option == 'object' && option
      if (!data) $this.data('flippanel', (data = new FlipPanel(this, options)))
      if (typeof option == 'string') data.change(option)
      if (typeof option == 'object') data.run(option)
    })
  }

  $.fn.flippanel.Constructor = FlipPanel

  $.fn.flippanel.defaults = {
    template: '<span class="layer top">' + 
                  '<span class="back side layer bottom-half">' + 
                    '<span class="adjust" data-next></span>' +
                  '</span>' +
                  '<span class="side layer top-half" data-current></span>' +
               '</span>' +
               '<span class="layer middle">' +
                  '<span class="side layer top-half" data-next></span>' +
               '</span>' +
               '<span class="layer bottom" data-current>' +
               '</span>' +
               '<hr/>',
    speed: 450
  }

}(window.jQuery);
