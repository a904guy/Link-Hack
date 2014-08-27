<?hh // strict

/*

   Link-Hack is a Hack Lang HHVM version of Amanpreet Singh's minimal PHP Router (https://github.com/apsdehal)

   Author: Andy Hawkins
   Website: (http://a904guy.com)

    __             __                   __
   |  |--.--------|  |--.-----.-----.--|  |
   |  _  |        |  _  |__ --|  _  |  _  |
   |_____|__|__|__|_____|_____|__   |_____|
   ----------------------------- |__| -----
   ANDY@BMBSQD.COM   -  WWW.BMBSQD.COM
   06/19/14

   Requirements:
   hhvm, [nginx/apache]

   Tested:
   Ubuntu 14.04, hhvm 3.2.0-dev+2014.06.18 (rel), nginx 1.4.6

   See README.md for instructions

 */

type Route = Map<TFunc, ?Tvars>;

class Link {

    private static Route $routes;
    private static Route $beforeFuncs;
    private static Route $afterFuncs;
    private static string $path = '/';
    private static string $method = 'get';
    private static array $regex = array('~{i}~', '~{s}~', '~{a}~'); // Can't Vector because of preg_replace
    private static array $replace = array('([\d]+)', '([ a-zA-Z\+]+)', '([\w-\+ ]+)');

    public static function all(Route $routes): void {

        self::IterateRoutes(self::$beforeFuncs);
        self::$routes = $routes;
        self::$method = strtolower($_SERVER['REQUEST_METHOD']);
        self::$path = urldecode(parse_url($_SERVER['REQUEST_URI'], PHP_URL_PATH));

        /* All Routes */
        foreach (self::$routes as $path => $route) {

            /* Static Routes */
            if ($path === self::$path) {
                self::IterateRoutes($route);
                break;
            }

            /* Static Dynamic Routes */
            if (false !== strpos($path,'{')) {
                $routePath = preg_replace(self::$regex, self::$replace, $path);
                if ($routePath !== $path) {
                    if (preg_match('~^/?' . $routePath . '/?$~', self::$path, $matches)) {
                        unset($matches['0']);
                        $route[array_keys($route)['0']] = $matches;
                        self::IterateRoutes($route);
                        break;
                    }
                }
            }

            /* Pure Regex Routes*/
            if (
                false !== strpos($path, '(')
                && preg_match('~^' . $path . '$~', self::$path, $matches)
            ) { // Has matching regex syntax
                unset($matches['0']);
                $route[array_keys($route)['0']] = $matches;
                self::IterateRoutes($route);
                break;
            }

        }

        self::IterateRoutes(self::$afterFuncs);
    }

    public static function addRegex(string $match, string $regex): void {
        self::$regex[] = $match;
        self::$replace[] = $regex;
    }

    public static function before(Route $r): void {
        self::$beforeFuncs = self::assignFunc($r, self::$beforeFuncs);
    }

    public static function after(Route $r): void {
        self::$afterFuncs = self::assignFunc($r, self::$afterFuncs);
    }

    public static function route(string $name, array $params) : string {
        $href = '';
        foreach (self::$routes as $path => $route) {
            foreach (array_keys($route) as $route_name) {
                if ($route_name !== $name) {
                    continue;
                }
                $href = $path;
                for ($i = 0; $i < count($params); $i++) {
                    $href = preg_replace('/{(.*?)}/', urlencode($params[$i]), $href, 1);
                }
            }
        }
        return (string) $href;
    }

    private static function assignFunc(Route $r, ?Route $d): Route {
        if (is_null($d)) {
            $d = $r;
        } else {
            foreach ($r as $k => $v) {
                $d[$k] = $v;
            }
        }
        return $d;
    }

    public static function handle404(): void {
        if (self::$routes.contains('404')) {
            self::IterateRoutes(Route('404', self::$routes.get('404')));
        } else {
            header($_SERVER['SERVER_PROTOCOL'] . ' 404 Not Found');
        }
    }

    public static function handle500(): void {
        if (self::$routes.contains('500')) {
            self::IterateRoutes(Route('500', self::$routes.get('500')));
        } else {
            header($_SERVER['SERVER_PROTOCOL'] . ' 500 Server Error');
        }
    }

    private static function IterateRoutes(?Route $funcs): void {
        if (!is_null($funcs)) {
            $funcs->mapWithKey(
                function ($k, $v) {
                    self::launch_class($k, $v);
                    self::launch_function($k,$v);
                }
            );
        }
    }

    private static function launch_class(string $k, mixed $v): void {
        if (class_exists($k)) {
            try {
                $name = new ReflectionClass($k);
                if (isset($v)) {
                    $class = $name->newInstanceArgs($v);
                } else {
                    $class = $name->newInstance();
                }
                // Check for RESTful methods in class
                if (method_exists($class, self::$method)) {
                    call_user_func_array(array($class, self::$method), $v);
                }
            } catch (Exception $e) {
                error_log(var_export($e, true));
                self::handle500();
            }
        }
    }

    private static function launch_function(string $k, mixed $v): void {
        try {
            if (function_exists($k) && is_array($v)) {
                call_user_func_array($k, $v);
            } elseif (function_exists($k)) {
                call_user_func($k);
            }
        } catch (Exception $e) {
            error_log(var_export($e, true));
            self::handle500();
        }
    }

}
