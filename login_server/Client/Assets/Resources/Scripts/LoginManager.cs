using System.Collections;
using System.Text;
using System.Text.RegularExpressions;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;
using UnityEngine.Events;
using UnityEngine.Networking;

public class LoginManager : MonoBehaviour 
{
	public static LoginManager Instance;

	public string serverIP;

	public GameObject loginPanel;
	public GameObject registerPanel;

	private InputField loginIdInputField;
	private InputField loginPasswordInputField;

	private InputField registerEmailInputField;
	private InputField registerIDInputField;
	private InputField registerPasswordInputField;

	private bool isRegisterCheckEmail;
	private bool isRegisterCheckID;

	private string idForResetPassword;	//패스워드 초기화를 위한 임시 ID 저장 변수

	void Awake()
	{
		Instance = this;
	}

	void Start () 
	{
		isRegisterCheckID = false;
		isRegisterCheckEmail = false;

		Screen.SetResolution (1280, 720, false);

		loginIdInputField = loginPanel.transform.FindChild("ID InputField").GetComponent<InputField>();
		loginPasswordInputField = loginPanel.transform.FindChild("Password InputField").GetComponent<InputField>();

		registerEmailInputField = registerPanel.transform.FindChild("Email InputField").GetComponent<InputField>();
		registerIDInputField = registerPanel.transform.FindChild("ID InputField").GetComponent<InputField>();
		registerPasswordInputField = registerPanel.transform.FindChild ("Password InputField").GetComponent<InputField> ();
	}

	void Update () 
	{
		
	}

    public bool IsValidEmail(string strIn) 
	{ 
		return Regex.IsMatch(strIn, @"^([\w-\.]+)@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.)|(([\w-]+\.)+))([a-zA-Z]{2,4}|[0-9]{1,3})(\]?)$"); 
	} 

	public void OnLogin()
	{
		StartCoroutine ("Login");
	}

	private IEnumerator Login()
	{		
		WWWForm form = new WWWForm();
		form.AddField ("id", loginIdInputField.text);
		form.AddField ("password", loginPasswordInputField.text);

		if (loginIdInputField.text == "" || loginPasswordInputField.text == "") 
		{
            GUIManager.Instance.ShowMessageBox ("ID 또는 비밀번호를 입력하세요.", null, MESSAGE_BOX_TYPE.SIMPLE);
            yield break;
			//StopCoroutine ("Login");
		}

		UnityWebRequest www;
		www = UnityWebRequest.Post(serverIP + "/login", form);

		yield return www.Send();

		Debug.Log (www.downloadHandler.text);

        if (www.downloadHandler.text.Substring(0, 2) == "ok")
        {
            DashboardManager.userToken = www.downloadHandler.text.Substring(3, 32);
            SceneManager.LoadScene("Lobby");
        }
		else if(www.downloadHandler.text == "already login")
		{
            GUIManager.Instance.ShowMessageBox ("이미 로그인 되어 있습니다.", null, MESSAGE_BOX_TYPE.SIMPLE);
		}
		else 
		{
            GUIManager.Instance.ShowMessageBox ("ID 또는 비밀번호가 존재하지 않습니다.", null, MESSAGE_BOX_TYPE.SIMPLE);
		}
	}

	public void OnForgotId()
	{
		GUIManager.Instance.ShowPrompt ("Your E-mail", "Enter Email...", PromptAnswerForgotIDEvent);
	}

	public void PromptAnswerForgotIDEvent(string command)
	{
		StartCoroutine ("FindID",command);
	}

	private IEnumerator FindID(string command)
	{
		WWWForm form = new WWWForm();
		form.AddField ("email", command);

		if (command == "") 
		{
            GUIManager.Instance.ShowMessageBox ("Email을 입력하세요.", null, MESSAGE_BOX_TYPE.SIMPLE);
            yield break;
			//StopCoroutine ("FindID");
		}

		UnityWebRequest www;
		www = UnityWebRequest.Post(serverIP + "/forgot/id", form);

		yield return www.Send();

		if (www.downloadHandler.text != "find fail") {
            GUIManager.Instance.ShowMessageBox ("ID : " + www.downloadHandler.text, null, MESSAGE_BOX_TYPE.SIMPLE);
		} 
		else 
		{
            GUIManager.Instance.ShowMessageBox ("찾는 ID가 없습니다.", null, MESSAGE_BOX_TYPE.SIMPLE);
		}
	}

	public void OnForgotPassword()
	{
		GUIManager.Instance.ShowPrompt ("Your ID", "Enter ID...", PromptAnswerForgotPasswordEvent);
	}

	public void PromptAnswerForgotPasswordEvent(string command)
	{
		StartCoroutine ("RequestToken",command);
	}

	private IEnumerator RequestToken(string command)
	{
		WWWForm form = new WWWForm();
		form.AddField ("id", command);
		idForResetPassword = command;

		if (command == "") 
		{
            GUIManager.Instance.ShowMessageBox ("ID를 입력하세요.", null, MESSAGE_BOX_TYPE.SIMPLE);
            yield break;
			//StopCoroutine ("RequestToken");
		}

		UnityWebRequest www;
		www = UnityWebRequest.Post(serverIP + "/forgot/password/request", form);

		yield return www.Send();

		Debug.Log (www.downloadHandler.text);

		if (www.downloadHandler.text == "sent email") 
			GUIManager.Instance.ShowPrompt("전송된 토큰을 입력하세요.", "Enter Token...", PromptAnswerInputTokenEvent);
		 else 
            GUIManager.Instance.ShowMessageBox ("이메일 에러 !!", null, MESSAGE_BOX_TYPE.SIMPLE);
	}

	public void PromptAnswerInputTokenEvent(string command)
	{
		StartCoroutine ("ResetPassword",command);
	}

	private IEnumerator ResetPassword(string command)
	{
		WWWForm form = new WWWForm();

		form.AddField ("uuid", command);

		UnityWebRequest www;
		www = UnityWebRequest.Post(serverIP + "/forgot/password/auth", form);

		yield return www.Send();

		Debug.Log (www.downloadHandler.text);

		if (www.downloadHandler.text == "success auth") 
		{
			GUIManager.Instance.ShowPrompt("새로운 비밀번호를 입력하세요.", "Enter New Password...", PromptAnswerNewPasswordEvent);
		} 
		else 
		{
            GUIManager.Instance.ShowMessageBox ("토큰이 틀렸습니다 !", null, MESSAGE_BOX_TYPE.SIMPLE);
		}
	}

	private void PromptAnswerNewPasswordEvent(string command)
	{
		StartCoroutine ("ChangePassword", command);
	}

	private IEnumerator ChangePassword(string command)
	{
		WWWForm form = new WWWForm();

		form.AddField ("id", idForResetPassword);
		form.AddField ("password", command);

		UnityWebRequest www;
		www = UnityWebRequest.Post(serverIP + "/forgot/password/reset", form);

		yield return www.Send();

		idForResetPassword = "";
		Debug.Log (www.downloadHandler.text);

		if (www.downloadHandler.text == "success")
            GUIManager.Instance.ShowMessageBox ("비밀번호가 변경되었습니다.", null, MESSAGE_BOX_TYPE.SIMPLE);
	}

	public void OnRegisterEmailCheck()
	{
		StartCoroutine ("RegisterEmailCheck");
	}

	private IEnumerator RegisterEmailCheck()
	{
		WWWForm form = new WWWForm ();
		form.AddField ("email", registerEmailInputField.text);

		if (registerEmailInputField.text == "") 
		{
            GUIManager.Instance.ShowMessageBox ("Email을 입력하세요.", null, MESSAGE_BOX_TYPE.SIMPLE);
            yield break;
			//StopCoroutine ("RegisterEmailCheck");
		}
            
        if (IsValidEmail(registerEmailInputField.text) == false)
        {
            GUIManager.Instance.ShowMessageBox ("올바른 Email이 아닙니다.", null, MESSAGE_BOX_TYPE.SIMPLE);
            yield break;
            //StopCoroutine ("RegisterEmailCheck");
        }

		UnityWebRequest www;
		www = UnityWebRequest.Post(serverIP + "/register/overlap/email", form);

		yield return www.Send();

		if (www.downloadHandler.text == "email ok") 
		{
			isRegisterCheckEmail = true;
            GUIManager.Instance.ShowMessageBox ("이메일 체크 성공 !", null, MESSAGE_BOX_TYPE.SIMPLE);
			registerEmailInputField.readOnly = true;
			registerEmailInputField.targetGraphic.color = Color.gray;
		} 
		else 
		{
			isRegisterCheckEmail = false;
            GUIManager.Instance.ShowMessageBox ("중복 된 이메일 입니다.", null, MESSAGE_BOX_TYPE.SIMPLE);
		}
	}

	public void OnRegisterIDCheck()
	{
		StartCoroutine ("RegisterIDCheck");
	}

	private IEnumerator RegisterIDCheck()
	{
		WWWForm form = new WWWForm ();
		form.AddField ("id", registerIDInputField.text);

		if (registerIDInputField.text == "") 
		{
            GUIManager.Instance.ShowMessageBox ("ID를 입력하세요.", null, MESSAGE_BOX_TYPE.SIMPLE);
            yield break;
			//StopCoroutine ("RegisterIDCheck");
		}

		UnityWebRequest www;
		www = UnityWebRequest.Post(serverIP + "/register/overlap/id", form);

		yield return www.Send();

		if (www.downloadHandler.text == "id ok")
		{
			isRegisterCheckID = true;
            GUIManager.Instance.ShowMessageBox ("ID 체크 성공 !", null, MESSAGE_BOX_TYPE.SIMPLE);
			registerIDInputField.readOnly = true;
			registerIDInputField.targetGraphic.color = Color.gray;
		} 
		else 
		{
			isRegisterCheckID = false;
            GUIManager.Instance.ShowMessageBox ("중복 된 ID 입니다.", null, MESSAGE_BOX_TYPE.SIMPLE);
		}
	}

	public void OnRegister()
	{
		StartCoroutine ("Register");
	}

	private IEnumerator Register()
	{
		if (isRegisterCheckEmail == true && isRegisterCheckID == true) 
		{
			WWWForm form = new WWWForm ();
			form.AddField ("id", registerIDInputField.text);
			form.AddField ("password", registerPasswordInputField.GetComponent<InputField> ().text);
			form.AddField ("email", registerEmailInputField.GetComponent<InputField> ().text);

			UnityWebRequest www;
			www = UnityWebRequest.Post (serverIP + "/register", form);

			yield return www.Send ();

			if (www.downloadHandler.text == "register ok") 
			{
				InputField[] registerInput = registerPanel.transform.GetComponentsInChildren<InputField> (true);

				foreach (var item in registerInput) 
				{
					item.text = "";
					item.readOnly = false;
				}

				isRegisterCheckEmail = false;
				isRegisterCheckID = false;

				registerEmailInputField.targetGraphic.color = Color.white;
				registerIDInputField.targetGraphic.color = Color.white;

                GUIManager.Instance.ShowMessageBox ("회원가입 성공 !", null, MESSAGE_BOX_TYPE.SIMPLE);
			}
		} 
		else 
		{
            GUIManager.Instance.ShowMessageBox ("ID 또는 Email의 중복을 체크하세요.", null, MESSAGE_BOX_TYPE.SIMPLE);
		}
	}
}
