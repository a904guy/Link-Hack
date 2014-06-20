## Link-Hack is a __Hack Lang HHVM version__ of Amanpreet Singh's __minimal__ PHP Router https://github.com/apsdehal/Link
A __minimal__ router for your webapps and APIs that effortlessly links all of your project. Its fast and to the point.

#Features
- RESTful routing
- Wildcards to help simplify writing routes
- Regex routes to unleash any possibility
- Named routes to help you create links easily
- Before and after routes function support
- Tested with Ubuntu 14.04, hhvm 3.2.0-dev+2014.06.18 (rel), nginx 1.4.6

# Dependencies

## Hack HHVM
http://docs.hhvm.com/manual/en/install.php

## Nginx
http://wiki.nginx.org/Install

# Install

## Manual Include

```php
	require("Link-Hack/src/Link.hh");
```

### Composer Coming Soon

# Config

## Nginx
```
server {
        listen 80 default_server;
        listen [::]:80 default_server ipv6only=on;

        root /path/Link-Hack/src/; # Changet to path of your environment
        server_name _;

	if (!-e $request_filename)
	{
	        rewrite ^/(.*)$ /Link_Debug.hh?/$1 last;
        	break;
	}

        location / {

        root /path/Link-Hack/src/; # Changet to path of your environment
        fastcgi_pass   127.0.0.1:9000; #Whatever HHVM daemon is set to run on.
        # or if you used a unix socket
        # fastcgi_pass   unix:/var/run/hhvm/hhvm.sock;
        fastcgi_index  Link.hh;
        fastcgi_param  SCRIPT_FILENAME $document_root$fastcgi_script_name;
        include        fastcgi_params;

        }

}
```

#Basics

##Simple Routing

```php

function routeMe(): void
{
	echo 'I am routed';
}

Link::all( Map{
	'/' => Map{'routeMe'=>False}
});
```

##Named Routing

In Link-Hack routes can be named and then further used generating links in a simple and elegant way.


```php

function nameMe() :void
{
	echo 'I am named';
}

Link::all( Map{
	'/named' => Map{'nameMe'=>False, 'Its my name'=>False}
});
```

Names to routes must be given as second argument in array while the first being the route handler.

### Usage

These named routes can be used in creating in hassle free links.

```html
	<a href="<?php echo Link::route('Its my name') ?>">Go to named route</a>
```

## Routing with classes

Link can handle classes as Route handler easily, but remember non-static class will be handled both on construct and RESTfully.

```php

$routes = Map{
	'/' => Map{'IndexController::getMeHome'=>False}, //Static function
	'/home' =>  Map{'HomeController'=>False}, //Class
	'/office' =>  Map{'OfficeController'=>False} // Class
});

Link::all($routes);

```

##RESTful routing

RESTful routing is a breeze for Link-Hack.

```php

class HomeController
{
	
	function get() :void
	{
		echo 'You have got to home :)';
	}

	function post() :void
	{
		echo 'You have posted to home';
	}

	function put() :void
	{
		echo 'You have put to home';
	}

	function delete() :void
	{
	    echo 'You have deleted the home :(';
	}
}

Link::all( Map{
	'/' => Map{'HomeController'=>False, 'HomeRoute'=>False}
});
```

# Dynamic Routes

## Regex Shorthands

Link-Hack supports numbers, string and alphanumeric wildcards which can be used as `{i} {s} {a}` respectively.

```php
$routes = Map{
	'/' => Map{'IndexController'=>False},
	'/{i}' => Map{'IndexController'=>False},
		//Parameter in place of {i} will be passed to IndexController
	'/posts/{a}/{i}/{s}' => Map{'PostsController'=>False}
};

Link::all($routes);
```

## Pure Regex

Link-Hack supports writing your own regex based routes.

```php
$routes = Map{
	'/regex/([\d]+)/([a-zA-Z]+)/([a-zA-Z]+)' => Map{'regexController'=>False}
};

Link::all($routes);
```

## Supplementary Handlers

Through Link-Hack, universal before and after handlers can be added, such that these are executed always before and after any route is routed. This can be done as follows:

```php

function universalBeforeHandler( $id ) :void
{
    echo 'Hello I occurred before with ' . $id . '\n';
}

function universalAfterHandler( $id ) :void
{
    if( $id )
        echo 'Hello I occurred after with ' . $id;
    else
        echo 'I simply occurred after';
}

function main() :void
{
    echo 'I simply occurred\n'
}

Link::before( Map{'universalBeforeHandler'=>array('12')} ); //If you want to pass parameters to them, pass them as arrays
Link::before( Map{'universalBeforeHandler'=>False} ); //else just don't specify them.

Link::all( Map{
    '/' => Map{'main'=>False}
})
```

Now go to '/' in your browser to find:

```sh
Hello I occurred before with 12

I simply occurred

I simply occurred after.
```

## Passing Parameters to Named Routes

You can pass parameters to named routes if the have wildcards in the route path, this will thus generate dynamic links through a single named route.

```php

function nameMe( $i, $s ) :void
{
	echo 'I am named and I have been passed ' . $i . $s ;
}

Link::all( Map{
	'/named/{i}/{s}' => Map{'nameMe'=>False, 'Its my name'=>False}
});
```

Now generate a link through Link

```php

echo Link::route( 'Its my name', array(1, 'Me') );
```

This in turn will generate `/named/1/Me` and the browser will return `I am named and I have been passed 1Me`

## [404,500] Errors

You should probably add a 404 handler to your routes Map, Link will take care of handling routes that are not found. In case, Link-Hack doesn't find a 404/500 route defined, it will just send the appropriate header. The 500 route will be executed if there is any exceptions thrown from the controllers/methods/functions/closure called.

```php

function notFound() :void
{
	echo 'This page is missing';
}

function errorFound() :void
{
	echo 'Oops, something went wrong, try again later';
}

function mainPage() :void
{
	throw new Exception('Meh? :(');
}

Link::all( Map{
	'/' => Map{'mainPage'=>False},
	'404' => Map{'notFound'=>False},
	'500' => Map{'errorFound'=>False}
});
```

# License
## Creative Commons Attribution-ShareAlike 3.0 Unported
http://creativecommons.org/licenses/by-sa/3.0/
