export function constrain(value: number, min: number, max: number) {
  return Math.min(max, Math.max(min, value));
}

export function sanitizeNumberString(input: string, maxDecimals?: number) {
  // Determine decimal separator for current locale
  const locale = navigator.language;
  const decimalSeparator = (1.1).toLocaleString(locale).substring(1, 2);
  let result = '';

  // Allow negative sign only at the start of the string
  const isNegative = input.startsWith('-');
  result = result.replaceAll('-', '');

  // Remove all non-digits, non-decimal separators and non-decimal separators
  result = input.replace(new RegExp(`[^0-9\\${decimalSeparator}]`, 'g'), '');

  // Replace all decimal separators except the first with nothing
  const decimalIndex = result.indexOf(decimalSeparator);
  result = result.replace(new RegExp(`\\${decimalSeparator}`, 'g'), (match, index) => (index > decimalIndex ? '' : match));

  // Limit the number of decimal digits
  const decimalPart = result.split(decimalSeparator)[1];
  if (maxDecimals && decimalPart && decimalPart.length > maxDecimals) {
    result = result.slice(0, decimalIndex + maxDecimals + 1);
  }

  // Remove trailing decimalSeparator edgecase
  if (decimalIndex >= 0 && maxDecimals === 0) {
    result = result.slice(0, decimalIndex);
  }

  return isNegative ? `-${result}` : result;
}

export function same(v1: number, v2:number) {
  return Math.abs(v1 - v2) < 0.0001;
}

export function getIndexInRange(desiredIndex: number | undefined, array: unknown[]): number | undefined {
  if (array.length === 0 || desiredIndex === undefined) {
    return undefined;
  }
  return Math.max(0, Math.min(array.length - 1, desiredIndex));
}
