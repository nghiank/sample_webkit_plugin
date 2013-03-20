function waitsFor( conditionCallback, completeCallback, interval )
{
	var waitToProceed = setInterval( function() {
		if ( conditionCallback && conditionCallback() )
		{			
			window.clearInterval(waitToProceed);
			completeCallback && completeCallback();
		}
	}, interval);
}

function clickContinue()
{
	$('#saveButton').trigger('click');
}

function selectLocation()
{
	$('#useLocation').trigger('click');
	waitsFor( function() { return $('#loading.loading-invisible').length > 0;}, clickContinue, 1000);
}

function searchAddress()
{
	$('#address').val("Singapore").change();
	$('#searchAddress').trigger('click');	
	
	waitsFor( function() { return document.getElementById('useLocation');}, selectLocation, 1000);
}

$(document).ready( function()
{
	waitsFor( 
		function() {
		return document.getElementById('address') && document.getElementById('btnedit');
		}, searchAddress, 1000);
});