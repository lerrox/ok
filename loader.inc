<?php
/**
 * Класс-обёртка над cURL.
 * Реализует работу с отправкой и получением данных в более удобном виде.
 * Необходим, пожалуй, для уменьшения кол-ва кода в приложении.
 */
class Loader 
{
	/**
	 * Url, на который будут отправляться данные
	 * @var string
	 */
	private $url			= null;

	/**
	 * Список параметров, которые необходимо передать удалённому скрипту
	 * @var array
	 */
	private $requestParams	= array();

	/**
	 * Список параметров загрузчика.
	 * Параметры, которые делегируются библиотеке cURL, поэтому ключи в данном
	 * массиве называются как константы настроек cURL, которые начинаются с CURLOPT_
	 * @var array
	 */
	private $loaderParams	= array(
		'CONNECTTIMEOUT'	=> 300,
		'TIMEOUT'			=> 300,
		'POST'				=> true,
		'RETURNTRANSFER'	=> true,
		'HEADER'			=> false,
		'VERBOSE'			=> true,
		'ENCODING'			=> '',
		'USERAGENT'			=> '',
		'FAILONERROR'		=> false,
		'HEADER_OUT'		=> true,
		'SSL_VERIFYPEER'	=> true,
		'SSL_VERIFYHOST'	=> 2
	);
	
	/**
     * необходимо ли выполнять http_build_query для $requestParams при отправке запроса
	 */
	private $escapeParams = true;


	/**
	 * Переводы ошибок cURL
	 * @var array
	 */
	private static $CurlErrorHash = array(
		CURLE_UNSUPPORTED_PROTOCOL => "The URL you passed to libcurl used a protocol that this libcurl does not support. The support might be a compile-time option that you didn't use, it can be a misspelled protocol string or just a protocol libcurl has no code for.",
		CURLE_FAILED_INIT => "Very early initialization code failed. This is likely to be an internal error or problem, or a resource problem where something fundamental couldn't get done at init time.",
		CURLE_URL_MALFORMAT => "Неверный формат URL.",
		CURLE_COULDNT_RESOLVE_PROXY => "Couldn't resolve proxy. The given proxy host could not be resolved.",
		CURLE_COULDNT_RESOLVE_HOST => "Couldn't resolve host. The given remote host was not resolved.",
		CURLE_COULDNT_CONNECT => "Не удалось подключиться к хосту или прокси-серверу.",
		CURLE_FTP_WEIRD_SERVER_REPLY => "After connecting to a FTP server, libcurl expects to get a certain reply back. This error code implies that it got a strange or bad reply. The given remote server is probably not an OK FTP server.",
		CURLE_FTP_WEIRD_PASS_REPLY => "After having sent the FTP password to the server, libcurl expects a proper reply. This error code indicates that an unexpected code was returned.",
		CURLE_FTP_WEIRD_PASV_REPLY => "libcurl failed to get a sensible result back from the server as a response to either a PASV or a EPSV command. The server is flawed.",
		CURLE_FTP_WEIRD_227_FORMAT => "FTP servers return a 227-line as a response to a PASV command. If libcurl fails to parse that line, this return code is passed back.",
		CURLE_FTP_CANT_GET_HOST => "An internal failure to lookup the host used for the new connection.",
		CURLE_PARTIAL_FILE => "A file transfer was shorter or larger than expected. This happens when the server first reports an expected transfer size, and then delivers data that doesn't match the previously given size.",
		CURLE_FTP_COULDNT_RETR_FILE => "This was either a weird reply to a 'RETR' command or a zero byte transfer complete.",
		CURLE_WRITE_ERROR => "An error occurred when writing received data to a local file, or an error was returned to libcurl from a write callback.",
		CURLE_READ_ERROR => "There was a problem reading a local file or an error returned by the read callback.",
		CURLE_OUT_OF_MEMORY => "A memory allocation request failed. This is serious badness and things are severely screwed up if this ever occurs.",
		CURLE_FTP_PORT_FAILED => "The FTP PORT command returned error. This mostly happens when you haven't specified a good enough address for libcurl to use. See CURLOPT_FTPPORT.",
		CURLE_FTP_COULDNT_USE_REST => "The FTP REST command returned error. This should never happen if the server is sane.",
		CURLE_HTTP_POST_ERROR => "This is an odd error that mainly occurs due to internal confusion.",
		CURLE_SSL_CONNECT_ERROR => "A problem occurred somewhere in the SSL/TLS handshake. You really want the error buffer and read the message there as it pinpoints the problem slightly more. Could be certificates (file formats, paths, permissions), passwords, and others.",
		CURLE_FILE_COULDNT_READ_FILE => "A file given with FILE:// couldn't be opened. Most likely because the file path doesn't identify an existing file. Did you check file permissions?",
		CURLE_LDAP_CANNOT_BIND => "LDAP cannot bind. LDAP bind operation failed.",
		CURLE_LDAP_SEARCH_FAILED => "LDAP search failed.",
		CURLE_FUNCTION_NOT_FOUND => "Function not found. A required zlib function was not found.",
		CURLE_ABORTED_BY_CALLBACK => "Aborted by callback. A callback returned 'abort' to libcurl.",
		CURLE_BAD_FUNCTION_ARGUMENT => "Internal error. A function was called with a bad parameter.",
		CURLE_TOO_MANY_REDIRECTS => "Too many redirects. When following redirects, libcurl hit the maximum amount. Set your limit with CURLOPT_MAXREDIRS.",
		CURLE_TELNET_OPTION_SYNTAX => "A telnet option string was Illegally formatted.",
		CURLE_GOT_NOTHING => "Nothing was returned from the server, and under the circumstances, getting nothing is considered an error.",
		CURLE_SSL_ENGINE_NOTFOUND => "The specified crypto engine wasn't found.",
		CURLE_SSL_ENGINE_SETFAILED => "Failed setting the selected SSL crypto engine as default!",
		CURLE_SEND_ERROR => "Failed sending network data.",
		CURLE_RECV_ERROR => "Failure with receiving network data.",
		CURLE_SSL_CERTPROBLEM => "problem with the local client certificate.",
		CURLE_SSL_CIPHER => "Couldn't use specified cipher.",
		CURLE_SSL_CACERT => "Peer certificate cannot be authenticated with known CA certificates.",
		CURLE_BAD_CONTENT_ENCODING => "Неизвестная кодировка передачи данных.",
		CURLE_LDAP_INVALID_URL => "Недействительный LDAP URL.",
		CURLE_FILESIZE_EXCEEDED => "Превышен максимальный размер файла.",
	);

	/**
	 * Ответ от удалённого скрипта, на который посылались данные
	 * @var string
	 */
	private $response		= null;

	/**
	 * Ошибка при работе с CURl
	 * @var string
	 */
	private $curlError		= null;

	/**
	 * @param string $url Url, на который будут отправляться запросы
	 * @param array $requestParams Параметры запроса
	 * @param array $loaderParams Параметры загрузчика
	 * @return void
	 */
	public function __construct($url, $requestParams = array(), $loaderParams = array(), $escapeParams = true)
	{
		$this->url = $url;
		$this->setRequestParams($requestParams);
		$this->setLoaderParams($loaderParams);
		$this->escapeParams = $escapeParams;
	}

	/**
	 * Отправка запроса на удалённый сервер. Результат запроса (ответ от сервера)
	 * сохраняется в объекте и может быть получен соответствующим методом.
	 * @param array $requestParams Параметры отправляемые в запросе
	 * @return bool
	 */
	public function send($requestParams = array())
	{
        global $Con;

        // Перед отправкой запроса закроем соединение с БД
        // чтобы не держалось долгое соединение, которое пока может использоваться
        // чем-то другим
        $Con->close();

		if (!empty($requestParams))
		{
			$this->setRequestParams($requestParams);
		}

		$this->setAdditionalRequestParams();

		$ch = curl_init();

		// Параметры загрузчика
		curl_setopt($ch, CURLOPT_CONNECTTIMEOUT,	$this->loaderParams['CONNECTTIMEOUT']);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER,	$this->loaderParams['RETURNTRANSFER']);
		curl_setopt($ch, CURLOPT_TIMEOUT,			$this->loaderParams['TIMEOUT']);
		curl_setopt($ch, CURLOPT_HEADER,			$this->loaderParams['HEADER']);
		curl_setopt($ch, CURLOPT_VERBOSE,			$this->loaderParams['VERBOSE']);
		curl_setopt($ch, CURLOPT_ENCODING,			$this->loaderParams['ENCODING']);
		curl_setopt($ch, CURLOPT_USERAGENT,			$this->loaderParams['USERAGENT']);
		curl_setopt($ch, CURLOPT_FAILONERROR,		$this->loaderParams['FAILONERROR']);
		curl_setopt($ch, CURLINFO_HEADER_OUT,		$this->loaderParams['HEADER_OUT']);
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER,	$this->loaderParams['SSL_VERIFYPEER']);
		curl_setopt($ch, CURLOPT_SSL_VERIFYHOST,	$this->loaderParams['SSL_VERIFYHOST']);

		// Параметры запроса
		curl_setopt($ch, CURLOPT_POST,				$this->loaderParams['POST']);
		if ($this->loaderParams['POST'])
		{
			// Отправка полей методом POST
			curl_setopt($ch, CURLOPT_URL,			$this->url);
			if ($this->escapeParams)
				curl_setopt($ch, CURLOPT_POSTFIELDS,	http_build_query($this->requestParams));
			else
				curl_setopt($ch, CURLOPT_POSTFIELDS,	$this->prepareRequestParams($this->requestParams));
		}
		else
		{
			// Отправка полей методом GET в строке запроса
			curl_setopt($ch, CURLOPT_URL, sprintf('%s?%s', $this->url, http_build_query($this->requestParams)));
		}

		// Отправка запроса
		$this->response = curl_exec($ch);

        // Ответ получен. Восстановим соединения с БД
        $Con->connect();

		if (false === $this->response)
		{
			$this->curlError = curl_errno($ch);
			curl_close($ch);
			return false;
		}
		else
		{
			curl_close($ch);
			return true;
		}
	}

	/**
	 * Преобразует массив массивов в массив со сложными ключами
	 * например, array('some' => array('other' => 'val'), 'bla' => 27)) в array('some[other]' => 'val', 'bla' => 27) 
	 * 
	 * @param array $params
	 * @return array
	 */
	private function prepareRequestParams($params, $level = 1)
	{
		$result = array();
		if (is_array($params))
		{
			foreach ($params as $key => $val)
			{
				if (is_array($val))
				{
					$val = $this->prepareRequestParams($val, $level + 1);
					foreach ($val as $innerKey => $innerVal)
					{
						if ($level == 1)
							$result[$key . $innerKey] = $innerVal;
						else
							$result['[' . $key . ']' . $innerKey] = $innerVal;
					}
				}
				else
				{
					if ($level == 1)
						$result[$key] = $val;
					else
						$result['[' . $key . ']'] = $val;
				}
			}
		}
		return $result;
	}
	
	/**
	 * Устанавливает новый набор параметров запроса
	 * @param array $newParams
	 * @return void
	 */
	public function setRequestParams($newParams)
	{
		if (is_array($newParams))
		{
			$this->requestParams = $newParams;
		}
	}

	/**
	 * Устанавливает параметры для загрузчика
	 * @param array $newParams
	 * @return void
	 */
	public function setLoaderParams($newParams)
	{
		if (is_array($newParams))
		{
			// Не будем просто присваивать новый набор параметров.
			// Вместо этого переберём переданный набор и присвоим существующим значениям,
			// чтобы не стирать значения параметров по умолчанию.
			foreach ($newParams as $key => $value)
			{
				if (isset($this->loaderParams[$key]))
				{
					$this->loaderParams[$key] = $value;
				}
			}
		}
	}

	/**
	 * Возвращает ответ от удалённого сервера после отправки запроса
	 * @return string
	 */
	public function getResponse()
	{
		return $this->response;
	}

	/**
	 * Возвращает текст ошибки, в случае ошибки при отправке данных
	 * @return string
	 */
	public function getLastError()
	{
        if (isset(self::$CurlErrorHash[$this->curlError]))
        {
            return self::$CurlErrorHash[$this->curlError];
        }
        else
        {
            return $this->curlError;
        }
	}

	/**
	 * добавляет к списку параметров запроса, дополнительные параметры
	 * @return void
	 */
	private function setAdditionalRequestParams()
	{
		$params = $this->requestParams;

		// добавим домен клиента, чтобы сервер знал откуда пришел запрос
		$params['CLIENT_DOMAIN'] = $_SERVER['HTTP_HOST'];

		$this->requestParams = $params;
	}
}
