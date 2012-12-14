<?php
/**
 * Клиент для FIX
 */
class OKClient {
    const SERVICE_URI = 'http://dmitry-b.open-gov.dev.i/';
    const LOGIN = 'FIX';
    const PASSWORD = '8mt3onroa';
    const AUTHOR = 'Фикс';
    const HOUSING_CATEGORY_ID = 23;

    protected $_userId;
    protected $_token;

    public function __construct() {
        $this -> doAuth();
    }
    
    public function getUserId()
    {
        return $this->_userId;
    }
    
    public function getToken()
    {
        return $this->_token;
    }

    /**
     * получаем токен и id юзера
     */
    public function doAuth() {
        $loader = new Loader(self::SERVICE_URI . 'sessions.json', array('login' => self::LOGIN, 'password' => self::PASSWORD, 'author' => self::AUTHOR, 'user_type' => 'housingsystem'));
        if (false === $loader -> send()) {
            // тут логи
            return FALSE;
        }

        $response = $loader -> getResponse();
        $user = json_decode($response, true);

        $this -> _userId = $user['data']['id'];
        $this -> _token = $user['data']['token'];
        return TRUE;
    }

    /**
     * метод работает только на тесте
     *  @param $message текст для заявки
     * @param $image uri какой нить картинки
     */
    public function create($message, $image, $id) {
        //TODO не забудьте указать картинку
        $loader = new Loader(
            self::SERVICE_URI . 'orders.json', 
            array(
                'order' => array(
                        'message' => $message, 
                        'title' => $message, 
                        'author' => 'Инкогнито И. И.', 
                        'category_id' => self::HOUSING_CATEGORY_ID, 
                        'lat' => '12.5', 
                        'long' => '156.7', 
                        'address' => 'Ямашева', 
                        'for_house_service' => $this->_userId, 
                        'housing_guid' =>  $id, 
                        'region' => 16, 
                        ), 
                    'user_id' => $this -> _userId, 
                    'token' => $this -> _token, 
                    'photo_count' => 2, 
                    'photo1' => "@" . $image, 
                    'photo2' => "@" . $image), 
                 array(), 
                 false);
        if (false === $loader -> send()) {
            // Ошибка при отправке данных
           // $loader -> getLastError();
            
        }
        return $loader -> getResponse();
    }

    /**
     * получаем сообщение
     *  @param $id id сообщения
     */
    public function get($id)
    {
         $loader = new Loader(
            self::SERVICE_URI . '/orders/' . $id . '.json',
                array(
                    'user_id' => $this->_userId,
                    'token' => $this->_token
                ),
                    array(
                        'POST' => false,
                    )
                );
         if (false === $loader->send())
         {
              // Ошибка при отправке данных
         }
         return json_decode($loader->getResponse(), true);
    }
    
    /**
     * получаем все сообщения
     *  
     */
    public function getAll()
    {
         $loader = new Loader(
            self::SERVICE_URI . '/orders.json',
                array(
                    'user_id' => $this->_userId,
                    'token' => $this->_token
                    //'page_num'=> номер страницы
                    //'page_size' => тзывов на странице
                ),
                array(
                    'POST' => false,
                ));
                
         if (false === $loader->send())
         {
            // Ошибка при отправке данных
         }  
         return json_decode($loader->getResponse());
    }

    /**
     * Меняем статус у сообщения
     *
     */
    public function changeStatus($messageID, $statusId, $comment) {
        $loader = new Loader(
            self::SERVICE_URI . '/orders/put/' . $messageID . '.json', 
            array(
                'order' => array(
                    'status' => $statusId, 
                    'gov_answer' => $comment), 
                 'user_id' => $this->_userId, 
                 'token' => $this->_token
                 ));
                 
        if (false === $loader -> send()) {
            // Ошибка при отправке данных
           // $loader -> getLastError();
        } 

        return json_decode($loader->getResponse(), true);
        
    }

}
