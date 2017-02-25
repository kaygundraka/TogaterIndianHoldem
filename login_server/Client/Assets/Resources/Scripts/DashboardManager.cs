using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Serialization;
using UnityEngine.SceneManagement;
using UnityEngine.UI;
using UnityEngine.Events;
using UnityEngine.Networking;

[System.Serializable]
class ProfileData
{
    public int win;
    public int lose;
    public int rating;
}

[System.Serializable]
class AccountData
{
    public string ID;
    public string email;
}

public class DashboardManager : MonoBehaviour 
{
    public static DashboardManager Instance;
    public static string userToken;

    public Transform profilePanel;
    public Transform accountPanel;

    void Awake()
    {
        Instance = this;
    }

	void Start () 
	{
        Debug.Log("Token : " + userToken);
        StartCoroutine("RequestProfile");
        StartCoroutine("RequestAccount");
	}

	void Update () 
	{
		
	}

    public void OnLogout()
    {
        StartCoroutine("Logout");
    }

    public void OnMembershipWithdrawal()
    {
        GUIManager.Instance.ShowMessageBox("정말 탈퇴하시겠습니까?", MembershipWithdrawal, MESSAGE_BOX_TYPE.OK_CANCLE);
    }

    private void MembershipWithdrawal()
    {
        StartCoroutine("StartMembershipWithdrawal");
    }

    public void OnUpdateEmail()
    {
        GUIManager.Instance.ShowPrompt("Update Email", "Enter Email...", UpdateEmail);
    }

    private void UpdateEmail(string command)
    {
        StartCoroutine("StartUpdateEmail");
    }

    private IEnumerator StartUpdateEmail(string command)
    {
        WWWForm form = new WWWForm();
        form.AddField ("userToken", userToken);
        form.AddField ("email", command);

        if (command == "") 
        {
            GUIManager.Instance.ShowMessageBox ("Email을 입력하세요.", null, MESSAGE_BOX_TYPE.SIMPLE);
            //StopCoroutine (corutine);
            yield break;
        }

        if (LoginManager.Instance.IsValidEmail(command) == false)
        {
            GUIManager.Instance.ShowMessageBox ("올바른 Email이 아닙니다.", null, MESSAGE_BOX_TYPE.SIMPLE);
            yield break;
            //StopCoroutine (corutine);
        }

        UnityWebRequest www;
        www = UnityWebRequest.Post(LoginManager.Instance.serverIP + "/dashboard/update/email", form);

        yield return www.Send();

        if (www.downloadHandler.text == "update email") {
            GUIManager.Instance.ShowMessageBox ("Email이 수정되었습니다.", null, MESSAGE_BOX_TYPE.SIMPLE);
        } 
        else 
        {
            GUIManager.Instance.ShowMessageBox ("Email을 다시 입력해주세요.", null, MESSAGE_BOX_TYPE.SIMPLE);
        }
    }

    private IEnumerator StartMembershipWithdrawal()
    {
        WWWForm form = new WWWForm();
        form.AddField ("userToken", userToken);

        UnityWebRequest www;
        www = UnityWebRequest.Post(LoginManager.Instance.serverIP + "/dashboard/withdrawal", form);

        yield return www.Send();

        if (www.downloadHandler.text == "success")
        {
            GUIManager.Instance.ShowMessageBox("탈퇴 되었습니다.", null, MESSAGE_BOX_TYPE.SIMPLE);
            SceneManager.LoadScene("Main");
        }
        else
            GUIManager.Instance.ShowMessageBox("회원 탈퇴 에러", null, MESSAGE_BOX_TYPE.SIMPLE);
    }

    private IEnumerator RequestProfile()
    {
        WWWForm form = new WWWForm();
        form.AddField ("userToken", userToken);

        UnityWebRequest www;
        www = UnityWebRequest.Post(LoginManager.Instance.serverIP + "/dashboard/profile", form);

        yield return www.Send();

        ProfileData data = JsonUtility.FromJson<ProfileData>(www.downloadHandler.text);

        profilePanel.FindChild("Win").GetComponent<Text>().text = "Win : " + data.win.ToString();
        profilePanel.FindChild("Lose").GetComponent<Text>().text = "Lose : " + data.lose.ToString();
        profilePanel.FindChild("Rating Score").GetComponent<Text>().text = "Rating Score : " + data.rating.ToString();
    }

    private IEnumerator RequestAccount()
    {
        WWWForm form = new WWWForm();
        form.AddField ("userToken", userToken);

        UnityWebRequest www;
        www = UnityWebRequest.Post(LoginManager.Instance.serverIP + "/dashboard/account", form);

        yield return www.Send();

        AccountData data = JsonUtility.FromJson<AccountData>(www.downloadHandler.text);

        accountPanel.FindChild("My ID").GetComponent<Text>().text = "ID : " + data.ID;
        accountPanel.FindChild("Email").GetComponent<Text>().text = "Email : " + data.email.ToString();
    }

    private IEnumerator Logout()
    {       
        WWWForm form = new WWWForm();
        form.AddField ("userToken", userToken);

        UnityWebRequest www;
        www = UnityWebRequest.Post(LoginManager.Instance.serverIP + "/dashboard/logout", form);

        yield return www.Send();

        Debug.Log(www.downloadHandler.text);
        SceneManager.LoadScene("Main");
    }
}
