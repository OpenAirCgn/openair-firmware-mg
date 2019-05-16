



function diff(obj=config, key="") {
  L(key)
  let differences = {}
  let el = document.querySelector(`${key}input`)
  if (el) {
    let value = el.value
    switch (typeof obj) {
      case "object":
        return _diffObject(obj, key)
        break;
      case "string":
        return value !== obj ? value : null
        break;
      case "number":
        let numValue = parseInt(value)
        if (!isNaN(numValue)) {
          return numValue !== obj ? numValue : null
        } else {
          return null; // throw?
        }
        break;
      case "boolean":
        return el.checked !== obj ? el.checked : null
        break;
      default:
        // err
        return null; // throw ?
    }
  }
  return null // does not exist as a ui element.
}

function _diffObject(obj, key) {
  let keys = Object.keys(obj)
  let diffs = {}
  let different = false
  for (let k of keys) {
    let obj2 = obj[k]
    let key2 = `${key}#${k} ` // space is significant
    let d = diff(obj2, key2)
    if (d !== null) {
      different = true
      diffs[k] = d
    }
  }
  return different ? diffs : null
}
